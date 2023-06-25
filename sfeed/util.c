#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "util.h"

/* print to stderr, print error message of errno and exit().
   Unlike BSD err() it does not prefix __progname */
__dead void
err(int exitstatus, const char *fmt, ...)
{
	va_list ap;
	int saved_errno;

	saved_errno = errno;

	if (fmt) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputs(": ", stderr);
	}
	fprintf(stderr, "%s\n", strerror(saved_errno));

	exit(exitstatus);
}

/* print to stderr and exit().
   Unlike BSD errx() it does not prefix __progname */
__dead void
errx(int exitstatus, const char *fmt, ...)
{
	va_list ap;

	if (fmt) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	fputs("\n", stderr);

	exit(exitstatus);
}

/* Handle read or write errors for a FILE * stream */
void
checkfileerror(FILE *fp, const char *name, int mode)
{
	if (mode == 'r' && ferror(fp))
		errx(1, "read error: %s", name);
	else if (mode == 'w' && (fflush(fp) || ferror(fp)))
		errx(1, "write error: %s", name);
}

/* strcasestr() included for portability */
char *
strcasestr(const char *h, const char *n)
{
	size_t i;

	if (!n[0])
		return (char *)h;

	for (; *h; ++h) {
		for (i = 0; n[i] && TOLOWER((unsigned char)n[i]) ==
		            TOLOWER((unsigned char)h[i]); ++i)
			;
		if (n[i] == '\0')
			return (char *)h;
	}

	return NULL;
}

/* Check if string has a non-empty scheme / protocol part. */
int
uri_hasscheme(const char *s)
{
	const char *p = s;

	for (; ISALPHA((unsigned char)*p) || ISDIGIT((unsigned char)*p) ||
		       *p == '+' || *p == '-' || *p == '.'; p++)
		;
	/* scheme, except if empty and starts with ":" then it is a path */
	return (*p == ':' && p != s);
}

/* Parse URI string `s` into an uri structure `u`.
   Returns 0 on success or -1 on failure */
int
uri_parse(const char *s, struct uri *u)
{
	const char *p = s;
	char *endptr;
	size_t i;
	long l;

	u->proto[0] = u->userinfo[0] = u->host[0] = u->port[0] = '\0';
	u->path[0] = u->query[0] = u->fragment[0] = '\0';

	/* protocol-relative */
	if (*p == '/' && *(p + 1) == '/') {
		p += 2; /* skip "//" */
		goto parseauth;
	}

	/* scheme / protocol part */
	for (; ISALPHA((unsigned char)*p) || ISDIGIT((unsigned char)*p) ||
		       *p == '+' || *p == '-' || *p == '.'; p++)
		;
	/* scheme, except if empty and starts with ":" then it is a path */
	if (*p == ':' && p != s) {
		if (*(p + 1) == '/' && *(p + 2) == '/')
			p += 3; /* skip "://" */
		else
			p++; /* skip ":" */

		if ((size_t)(p - s) >= sizeof(u->proto))
			return -1; /* protocol too long */
		memcpy(u->proto, s, p - s);
		u->proto[p - s] = '\0';

		if (*(p - 1) != '/')
			goto parsepath;
	} else {
		p = s; /* no scheme format, reset to start */
		goto parsepath;
	}

parseauth:
	/* userinfo (username:password) */
	i = strcspn(p, "@/?#");
	if (p[i] == '@') {
		if (i >= sizeof(u->userinfo))
			return -1; /* userinfo too long */
		memcpy(u->userinfo, p, i);
		u->userinfo[i] = '\0';
		p += i + 1;
	}

	/* IPv6 address */
	if (*p == '[') {
		/* bracket not found, host too short or too long */
		i = strcspn(p, "]");
		if (p[i] != ']' || i < 3)
			return -1;
		i++; /* including "]" */
	} else {
		/* domain / host part, skip until port, path or end. */
		i = strcspn(p, ":/?#");
	}
	if (i >= sizeof(u->host))
		return -1; /* host too long */
	memcpy(u->host, p, i);
	u->host[i] = '\0';
	p += i;

	/* port */
	if (*p == ':') {
		p++;
		if ((i = strcspn(p, "/?#")) >= sizeof(u->port))
			return -1; /* port too long */
		memcpy(u->port, p, i);
		u->port[i] = '\0';
		/* check for valid port: range 1 - 65535, may be empty */
		errno = 0;
		l = strtol(u->port, &endptr, 10);
		if (i && (errno || *endptr || l <= 0 || l > 65535))
			return -1;
		p += i;
	}

parsepath:
	/* path */
	if ((i = strcspn(p, "?#")) >= sizeof(u->path))
		return -1; /* path too long */
	memcpy(u->path, p, i);
	u->path[i] = '\0';
	p += i;

	/* query */
	if (*p == '?') {
		p++;
		if ((i = strcspn(p, "#")) >= sizeof(u->query))
			return -1; /* query too long */
		memcpy(u->query, p, i);
		u->query[i] = '\0';
		p += i;
	}

	/* fragment */
	if (*p == '#') {
		p++;
		if ((i = strlen(p)) >= sizeof(u->fragment))
			return -1; /* fragment too long */
		memcpy(u->fragment, p, i);
		u->fragment[i] = '\0';
	}

	return 0;
}

/* Transform and try to make the URI `u` absolute using base URI `b` into `a`.
   Follows some of the logic from "RFC 3986 - 5.2.2. Transform References".
   Returns 0 on success, -1 on error or truncation. */
int
uri_makeabs(struct uri *a, struct uri *u, struct uri *b)
{
	char *p;
	int c;

	strlcpy(a->fragment, u->fragment, sizeof(a->fragment));

	if (u->proto[0] || u->host[0]) {
		strlcpy(a->proto, u->proto[0] ? u->proto : b->proto, sizeof(a->proto));
		strlcpy(a->host, u->host, sizeof(a->host));
		strlcpy(a->userinfo, u->userinfo, sizeof(a->userinfo));
		strlcpy(a->host, u->host, sizeof(a->host));
		strlcpy(a->port, u->port, sizeof(a->port));
		strlcpy(a->path, u->path, sizeof(a->path));
		strlcpy(a->query, u->query, sizeof(a->query));
		return 0;
	}

	strlcpy(a->proto, b->proto, sizeof(a->proto));
	strlcpy(a->host, b->host, sizeof(a->host));
	strlcpy(a->userinfo, b->userinfo, sizeof(a->userinfo));
	strlcpy(a->host, b->host, sizeof(a->host));
	strlcpy(a->port, b->port, sizeof(a->port));

	if (!u->path[0]) {
		strlcpy(a->path, b->path, sizeof(a->path));
	} else if (u->path[0] == '/') {
		strlcpy(a->path, u->path, sizeof(a->path));
	} else {
		a->path[0] = (b->host[0] && b->path[0] != '/') ? '/' : '\0';
		a->path[1] = '\0';

		if ((p = strrchr(b->path, '/'))) {
			c = *(++p);
			*p = '\0'; /* temporary NUL-terminate */
			if (strlcat(a->path, b->path, sizeof(a->path)) >= sizeof(a->path))
				return -1;
			*p = c; /* restore */
		}
		if (strlcat(a->path, u->path, sizeof(a->path)) >= sizeof(a->path))
			return -1;
	}

	if (u->path[0] || u->query[0])
		strlcpy(a->query, u->query, sizeof(a->query));
	else
		strlcpy(a->query, b->query, sizeof(a->query));

	return 0;
}

int
uri_format(char *buf, size_t bufsiz, struct uri *u)
{
	return snprintf(buf, bufsiz, "%s%s%s%s%s%s%s%s%s%s%s%s",
		u->proto,
		u->userinfo[0] ? u->userinfo : "",
		u->userinfo[0] ? "@" : "",
		u->host,
		u->port[0] ? ":" : "",
		u->port,
		u->host[0] && u->path[0] && u->path[0] != '/' ? "/" : "",
		u->path,
		u->query[0] ? "?" : "",
		u->query,
		u->fragment[0] ? "#" : "",
		u->fragment);
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

	/* NOTE: the type long long supports the 64-bit range. If time_t is
	   64-bit it is "2038-ready", otherwise it is truncated/wrapped. */
	if (t)
		*t = (time_t)l;

	return 0;
}

time_t
getcomparetime(void)
{
	time_t now, t;
	char *p;

	if ((now = time(NULL)) == (time_t)-1)
		return (time_t)-1;

	if ((p = getenv("SFEED_NEW_AGE"))) {
		if (strtotime(p, &t) == -1)
			return (time_t)-1;
		return now - t;
	}

	return now - 86400; /* 1 day is old news */
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
		default:   putc(*s, fp);
		}
	}
}

/* print `len` columns of characters. If string is shorter pad the rest with
 * characters `pad`. */
void
printutf8pad(FILE *fp, const char *s, size_t len, int pad)
{
	wchar_t wc;
	size_t col = 0, i, slen;
	int inc, rl, w;

	if (!len)
		return;

	slen = strlen(s);
	for (i = 0; i < slen; i += inc) {
		inc = 1; /* next byte */
		if ((unsigned char)s[i] < 32) {
			continue; /* skip control characters */
		} else if ((unsigned char)s[i] >= 127) {
			rl = mbtowc(&wc, s + i, slen - i < 4 ? slen - i : 4);
			inc = rl;
			if (rl < 0) {
				mbtowc(NULL, NULL, 0); /* reset state */
				inc = 1; /* invalid, seek next byte */
				w = 1; /* replacement char is one width */
			} else if ((w = wcwidth(wc)) == -1) {
				continue;
			}

			if (col + w > len || (col + w == len && s[i + inc])) {
				fputs(PAD_TRUNCATE_SYMBOL, fp); /* ellipsis */
				col++;
				break;
			} else if (rl < 0) {
				fputs(UTF_INVALID_SYMBOL, fp); /* replacement */
				col++;
				continue;
			}
			fwrite(&s[i], 1, rl, fp);
			col += w;
		} else {
			/* optimization: simple ASCII character */
			if (col + 1 > len || (col + 1 == len && s[i + 1])) {
				fputs(PAD_TRUNCATE_SYMBOL, fp); /* ellipsis */
				col++;
				break;
			}
			putc(s[i], fp);
			col++;
		}

	}
	for (; col < len; ++col)
		putc(pad, fp);
}
