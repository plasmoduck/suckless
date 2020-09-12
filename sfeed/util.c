#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "util.h"

int
parseuri(const char *s, struct uri *u, int rel)
{
	const char *p = s, *b;
	char *endptr = NULL;
	size_t i;
	unsigned long l;

	u->proto[0] = u->host[0] = u->path[0] = u->port[0] = '\0';
	if (!*s)
		return 0;

	/* prefix is "//", don't read protocol, skip to domain parsing */
	if (!strncmp(p, "//", 2)) {
		p += 2; /* skip "//" */
	} else {
		/* protocol part */
		for (p = s; *p && (isalpha((unsigned char)*p) || isdigit((unsigned char)*p) ||
			       *p == '+' || *p == '-' || *p == '.'); p++)
			;
		if (!strncmp(p, "://", 3)) {
			if ((size_t)(p - s) >= sizeof(u->proto))
				return -1; /* protocol too long */
			memcpy(u->proto, s, p - s);
			u->proto[p - s] = '\0';
			p += 3; /* skip "://" */
		} else {
			p = s; /* no protocol format, set to start */
			/* relative url: read rest as path, else as domain */
			if (rel)
				goto readpath;
		}
	}
	/* IPv6 address */
	if (*p == '[') {
		/* bracket not found or host too long */
		if (!(b = strchr(p, ']')) || (size_t)(b - p) < 3 ||
		    (size_t)(b - p) >= sizeof(u->host))
			return -1;
		memcpy(u->host, p, b - p + 1);
		u->host[b - p + 1] = '\0';
		p = b + 1;
	} else {
		/* domain / host part, skip until port, path or end. */
		if ((i = strcspn(p, ":/")) >= sizeof(u->host))
			return -1; /* host too long */
		memcpy(u->host, p, i);
		u->host[i] = '\0';
		p = &p[i];
	}
	/* port */
	if (*p == ':') {
		if ((i = strcspn(++p, "/")) >= sizeof(u->port))
			return -1; /* port too long */
		memcpy(u->port, p, i);
		u->port[i] = '\0';
		/* check for valid port: range 1 - 65535 */
		errno = 0;
		l = strtoul(u->port, &endptr, 10);
		if (errno || u->port[0] == '\0' || *endptr ||
		    !l || l > 65535)
			return -1;
		p = &p[i];
	}
readpath:
	if (u->host[0]) {
		p = &p[strspn(p, "/")];
		strlcpy(u->path, "/", sizeof(u->path));
	} else {
		/* absolute uri must have a host specified */
		if (!rel)
			return -1;
	}
	/* treat truncation as an error */
	if (strlcat(u->path, p, sizeof(u->path)) >= sizeof(u->path))
		return -1;
	return 0;
}

static int
encodeuri(char *buf, size_t bufsiz, const char *s)
{
	static const char *table = "0123456789ABCDEF";
	size_t i, b;

	for (i = 0, b = 0; s[i]; i++) {
		if ((unsigned char)s[i] <= ' ' ||
		    (unsigned char)s[i] >= 127) {
			if (b + 3 >= bufsiz)
				return -1;
			buf[b++] = '%';
			buf[b++] = table[((unsigned char)s[i] >> 4) & 15];
			buf[b++] = table[(unsigned char)s[i] & 15];
		} else if (b < bufsiz) {
			buf[b++] = s[i];
		} else {
			return -1;
		}
	}
	if (b >= bufsiz)
		return -1;
	buf[b] = '\0';

	return 0;
}

/* Get absolute uri; if `link` is relative use `base` to make it absolute.
 * the returned string in `buf` is uri encoded, see: encodeuri(). */
int
absuri(char *buf, size_t bufsiz, const char *link, const char *base)
{
	struct uri ulink, ubase;
	char tmp[4096], *host, *p, *port;
	int c, r;
	size_t i;

	buf[0] = '\0';
	if (parseuri(base, &ubase, 0) == -1 ||
	    parseuri(link, &ulink, 1) == -1 ||
	    (!ulink.host[0] && !ubase.host[0]))
		return -1;

	if (!strncmp(link, "//", 2)) {
		host = ulink.host;
		port = ulink.port;
	} else {
		host = ulink.host[0] ? ulink.host : ubase.host;
		port = ulink.port[0] ? ulink.port : ubase.port;
	}
	r = snprintf(tmp, sizeof(tmp), "%s://%s%s%s",
		ulink.proto[0] ?
			ulink.proto :
			(ubase.proto[0] ? ubase.proto : "http"),
		host,
		port[0] ? ":" : "",
		port);
	if (r < 0 || (size_t)r >= sizeof(tmp))
		return -1; /* error or truncation */

	/* relative to root */
	if (!ulink.host[0] && ulink.path[0] != '/') {
		/* relative to base url path */
		if (ulink.path[0]) {
			if ((p = strrchr(ubase.path, '/'))) {
				/* temporary null-terminate */
				c = *(++p);
				*p = '\0';
				i = strlcat(tmp, ubase.path, sizeof(tmp));
				*p = c; /* restore */
				if (i >= sizeof(tmp))
					return -1;
			}
		} else if (strlcat(tmp, ubase.path, sizeof(tmp)) >=
		           sizeof(tmp)) {
			return -1;
		}
	}
	if (strlcat(tmp, ulink.path, sizeof(tmp)) >= sizeof(tmp))
		return -1;

	return encodeuri(buf, bufsiz, tmp);
}

/* Splits fields in the line buffer by replacing TAB separators with NUL ('\0')
 * terminators and assign these fields as pointers. If there are less fields
 * than expected then the field is an empty string constant. */
void
parseline(char *line, char *fields[FieldLast])
{
	char *prev, *s;
	size_t i;

	for (prev = line, i = 0;
	    (s = strchr(prev, '\t')) && i < FieldLast - 1;
	    i++) {
		*s = '\0';
		fields[i] = prev;
		prev = s + 1;
	}
	fields[i++] = prev;
	/* make non-parsed fields empty. */
	for (; i < FieldLast; i++)
		fields[i] = "";
}

/* Parse time to time_t, assumes time_t is signed, ignores fractions. */
int
strtotime(const char *s, time_t *t)
{
	long long l;
	char *e;

	errno = 0;
	l = strtoll(s, &e, 10);
	if (errno || *s == '\0' || *e)
		return -1;
	/* NOTE: assumes time_t is 64-bit on 64-bit platforms:
	         long long (atleast 32-bit) to time_t. */
	if (t)
		*t = (time_t)l;

	return 0;
}

/* Escape characters below as HTML 2.0 / XML 1.0. */
void
xmlencode(const char *s, FILE *fp)
{
	for (; *s; ++s) {
		switch (*s) {
		case '<':  fputs("&lt;",   fp); break;
		case '>':  fputs("&gt;",   fp); break;
		case '\'': fputs("&#39;",  fp); break;
		case '&':  fputs("&amp;",  fp); break;
		case '"':  fputs("&quot;", fp); break;
		default:   fputc(*s, fp);
		}
	}
}

/* print `len' columns of characters. If string is shorter pad the rest with
 * characters `pad`. */
void
printutf8pad(FILE *fp, const char *s, size_t len, int pad)
{
	wchar_t wc;
	size_t col = 0, i, slen;
	int rl, w;

	if (!len)
		return;

	slen = strlen(s);
	for (i = 0; i < slen; i += rl) {
		rl = w = 1;
		if ((unsigned char)s[i] < 32)
			continue;
		if ((unsigned char)s[i] >= 127) {
			if ((rl = mbtowc(&wc, s + i, slen - i < 4 ? slen - i : 4)) <= 0)
				break;
			if ((w = wcwidth(wc)) == -1)
				continue;
		}
		if (col + w > len || (col + w == len && s[i + rl])) {
			fputs("\xe2\x80\xa6", fp);
			col++;
			break;
		}
		fwrite(&s[i], 1, rl, fp);
		col += w;
	}
	for (; col < len; ++col)
		putc(pad, fp);
}
