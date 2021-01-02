#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* DEBUG: used for clock_gettime, remove later */
#include <wchar.h>
#include <wctype.h>

#include "adblock.h"

/* String data / memory pool */
typedef struct string {
	char   *data;   /* data */
	size_t  datasz; /* allocated size */
	size_t  len;    /* current string length */
} String;

struct filterdomain {
	char *domain;
	int inverse;
	struct filterdomain *next;
};

struct filterrule {
	/* type: match mask, must be atleast 32-bit, see FilterType enum */
	unsigned long block;
	int matchbegin;
	int matchend;
	/* is exception rule: prefix @@ for ABP or #@# for CSS */
	int isexception;
	const char *css; /* if non-NULL is CSS rule / hide element rule */
	const char *uri;
	struct filterdomain *domains;
	struct filterrule *next;
};

enum {
	FilterTypeScript       = 1 << 0,
	FilterTypeImage        = 1 << 1,
	FilterTypeCSS          = 1 << 2,
	FilterTypeObject       = 1 << 3,
	FilterTypeXHR          = 1 << 4,
	FilterTypeObjectSub    = 1 << 5,
	FilterTypeSubDoc       = 1 << 6,
	FilterTypePing         = 1 << 7,
	FilterTypeDocument     = 1 << 8,
	FilterTypeElemHide     = 1 << 9,
	FilterTypeOther        = 1 << 10,
	FilterTypeGenericHide  = 1 << 11,
	FilterTypeGenericBlock = 1 << 12,
	FilterTypeMatchCase    = 1 << 13,
};

struct filtertype {
	/* `type` must be atleast 32-bit, see FilterType enum */
	unsigned long type;
	char *name;
	size_t namelen;
	int allowinverse;
	int allownormal;
	int onlyexception;
	int (*fn)(struct filterrule *, char *);
};

static int parsedomainsoption(struct filterrule *, char *);

#define STRP(s) s,sizeof(s)-1

static struct filtertype filtertypes[] = {
	/* NOTE: options with 'type' = 0 are silently ignored and treated as
	 *       requests for now */
	{ 0,                      STRP("collapse"),          1, 1, 0, NULL },
	{ FilterTypeDocument,     STRP("document"),          1, 0, 1, NULL },
	{ 0,                      STRP("domain"),            0, 1, 0,
	                             /* domain=... */  &parsedomainsoption },
	{ 0,                      STRP("donottrack"),        1, 1, 0, NULL },
	{ FilterTypeElemHide,     STRP("elemhide"),          0, 0, 1, NULL },
	{ 0,                      STRP("font"),              1, 1, 0, NULL },
	{ FilterTypeGenericBlock, STRP("genericblock"),      1, 1, 1, NULL },
	{ FilterTypeGenericHide,  STRP("generichide"),       1, 1, 1, NULL },
	{ FilterTypeImage,        STRP("image"),             1, 1, 0, NULL },
	{ FilterTypeMatchCase,    STRP("match-case"),        1, 1, 0, NULL },
	{ 0,                      STRP("media"),             1, 1, 0, NULL },
	{ FilterTypeObject,       STRP("object"),            1, 1, 0, NULL },
	{ FilterTypeObjectSub,    STRP("object-subrequest"), 1, 1, 0, NULL },
	{ FilterTypeOther,        STRP("other"),             1, 1, 0, NULL },
	{ FilterTypePing,         STRP("ping"),              1, 1, 0, NULL },
	{ 0,                      STRP("popup"),             1, 1, 0, NULL },
	{ FilterTypeScript,       STRP("script"),            1, 1, 0, NULL },
	{ FilterTypeCSS,          STRP("stylesheet"),        1, 1, 0, NULL },
	{ FilterTypeSubDoc,       STRP("subdocument"),       1, 1, 0, NULL },
	{ 0,                      STRP("third-party"),       1, 1, 0, NULL },
	{ FilterTypeXHR,          STRP("xmlhttprequest"),    1, 1, 0, NULL },
	/* NOTE: site-key not supported */
};

static String globalcss;
static struct filterrule *rules;

static void
weprintf(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "surf-adblock: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void *
wecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		weprintf("calloc: %s\n", strerror(errno));

	return p;
}

static char *
westrndup(const char *s, size_t n)
{
	char *p;

	if (!(p = strndup(s, n)))
		weprintf("strndup: %s\n", strerror(errno));
	return p;
}

static char *
westrdup(const char *s)
{
	char *p;

	if (!(p = strdup(s)))
		weprintf("strdup: %s\n", strerror(errno));

	return p;
}

static size_t
string_buffer_realloc(String *s, size_t newsz)
{
	char *tmp;
	size_t allocsz;

	for (allocsz = 64; allocsz <= newsz; allocsz *= 2)
		;
	if (!(tmp = realloc(s->data, allocsz))) {
		weprintf("realloc: %s\n", strerror(errno));
	} else {
		s->data   = tmp;
		s->datasz = allocsz;
	}

	return s->datasz;
}

static size_t
string_append(String *s, const char *data, size_t len)
{
	size_t newlen;

	if (!len)
		return len;

	newlen = s->len + len;
	/* check if allocation is necesary, don't shrink buffer,
	 * should be more than datasz ofcourse. */
	if (newlen >= s->datasz) {
		if (string_buffer_realloc(s, newlen + 1) <= newlen)
			return 0;
	}
	memcpy(s->data + s->len, data, len);
	s->len = newlen;
	s->data[s->len] = '\0';

	return len;
}

#define END          0
#define UNMATCHABLE -2
#define CARET       -3
#define STAR        -4

static int
str_next(const char *str, size_t n, size_t *step)
{
	if (!n) {
		*step = 0;
		return 0;
	}
	if (str[0] >= 128U) {
		wchar_t wc;
		int k = mbtowc(&wc, str, n);
		if (k<0) {
			*step = 1;
			return -1;
		}
		*step = k;
		return wc;
	}
	*step = 1;

	return str[0];
}

static int
pat_next(const char *pat, size_t m, size_t *step)
{
	int esc = 0;

	if (!m || !*pat) {
		*step = 0;
		return END;
	}
	*step = 1;
	if (pat[0]=='\\' && pat[1]) {
		*step = 2;
		pat++;
		esc = 1;
		goto escaped;
	}
	if (pat[0]=='^')
		return CARET;
	if (pat[0] == '*')
		return STAR;
escaped:
	if (pat[0] >= 128U) {
		wchar_t wc;
		int k = mbtowc(&wc, pat, m);
		if (k<0) {
			*step = 0;
			return UNMATCHABLE;
		}
		*step = k + esc;
		return wc;
	}
	return pat[0];
}

static int
casefold(int k)
{
	int c;

	/* optimization: -2% last measured.
	if ((unsigned)k < 128) {
		c = toupper(k);
		return c == k ? tolower(k) : c;
	}*/
	c = towupper(k);
	return c == k ? towlower(k) : c;
}

/* match() based on musl-libc fnmatch:
   https://git.musl-libc.org/cgit/musl/tree/src/regex/fnmatch.c */
static int
match(const char *pat, const char *str, int fcase)
{
	size_t m = -1, n = -1;
	const char *p, *ptail, *endpat;
	const char *s, *stail, *endstr;
	size_t pinc, sinc, tailcnt=0;
	int c, k, kfold;

	for (;;) {
		switch ((c = pat_next(pat, m, &pinc))) {
		case UNMATCHABLE:
			return 1;
		case STAR:
			pat++;
			m--;
			break;
		case CARET:
			k = str_next(str, n, &sinc);
			if (k <= 0)
				return (c==END) ? 0 : 1;
			str += sinc;
			n -= sinc;
			if (k != '?' && k != '/')
				return 1;
			pat++;
			m--;
			break;
		default:
			k = str_next(str, n, &sinc);
			if (k <= 0)
				return (c==END) ? 0 : 1;
			str += sinc;
			n -= sinc;
			kfold = fcase ? casefold(k) : k;
			if (k != c && kfold != c)
				return 1;
			pat+=pinc;
			m-=pinc;
			continue;
		}
		break;
	}

	/* Compute real pat length if it was initially unknown/-1 */
	m = strnlen(pat, m);
	endpat = pat + m;

	/* Find the last * in pat and count chars needed after it */
	for (p=ptail=pat; p<endpat; p+=pinc) {
		switch (pat_next(p, endpat-p, &pinc)) {
		case UNMATCHABLE:
			return 1;
		case STAR:
			tailcnt=0;
			ptail = p+1;
			break;
		default:
			tailcnt++;
			break;
		}
	}

	/* Past this point we need not check for UNMATCHABLE in pat,
	 * because all of pat has already been parsed once. */

	/* Compute real str length if it was initially unknown/-1 */
	n = strnlen(str, n);
	endstr = str + n;
	if (n < tailcnt) return 1;

	/* Find the final tailcnt chars of str, accounting for UTF-8.
	 * On illegal sequences we may get it wrong, but in that case
	 * we necessarily have a matching failure anyway. */
	for (s=endstr; s>str && tailcnt; tailcnt--) {
		if (s[-1] < 128U || MB_CUR_MAX==1) s--;
		else while ((unsigned char)*--s-0x80U<0x40 && s>str);
	}
	if (tailcnt) return 1;
	stail = s;

	/* Check that the pat and str tails match */
	p = ptail;
	for (;;) {
		c = pat_next(p, endpat-p, &pinc);
		p += pinc;
		if ((k = str_next(s, endstr-s, &sinc)) <= 0) {
			if (c != END) return 1;
			break;
		}
		s += sinc;
		if (c == CARET) {
			if  (k != '/' && k != '?')
				return 1;
		} else {
			kfold = fcase ? casefold(k) : k;
			if (k != c && kfold != c)
				return 1;
		}
	}

	/* We're all done with the tails now, so throw them out */
	endstr = stail;
	endpat = ptail;

	/* Match pattern components until there are none left */
	while (pat<endpat) {
		p = pat;
		s = str;
		for (;;) {
			c = pat_next(p, endpat-p, &pinc);
			p += pinc;
			/* Encountering * completes/commits a component */
			if (c == STAR) {
				pat = p;
				str = s;
				break;
			}
			k = str_next(s, endstr-s, &sinc);
			if (!k)
				return 1;
			s += sinc;
			if (c == CARET) {
				if (k != '/' && k != '?')
					break;
			} else {
				kfold = fcase ? casefold(k) : k;
				if (k != c && kfold != c)
					break;
			}

		}
		if (c == STAR) continue;
		/* If we failed, advance str, by 1 char if it's a valid
		 * char, or past all invalid bytes otherwise. */
		k = str_next(str, endstr-str, &sinc);
		if (k > 0) str += sinc;
		else for (str++; str_next(str, endstr-str, &sinc)<0; str++);
	}

	return 0;
}

/*
domain=...   if domain is prefixed with ~, ignore.
multiple domains can be separated with |
*/
static int
parsedomains(const char *s, int sep, struct filterdomain **head)
{
	struct filterdomain *d, *last = *head = NULL;
	char *p;
	int inverse;

	do {
		inverse = 0;
		if (*s == '~') {
			inverse = !inverse;
			s++;
		}
		if (!*s || *s == sep)
			break;

		if (!(d = wecalloc(1, sizeof(struct filterdomain))))
			return -1;
		if ((p = strchr(s, sep))) { /* TODO: should not contain ',' */
			d->domain = westrndup(s, p - s);
			s = p + 1;
		} else {
			d->domain = westrdup(s);
		}
		if (!d->domain)
			return -1;
		d->inverse = inverse;

		if (!*head)
			*head = last = d;
		else
			last = last->next = d;
	} while (p);

	return (*head != NULL);
}

static int
parsedomainselement(struct filterrule *f, char *s)
{
	struct filterdomain *d, *last;

	for (last = f->domains; last && last->next; last = last->next)
		;

	if (parsedomains(s, ',', &d) < 0)
		return -1;
	if (last)
		last->next = d;
	else
		f->domains = d;

	return (d != NULL);
}

static int
parsedomainsoption(struct filterrule *f, char *s)
{
	struct filterdomain *d, *last;

	for (last = f->domains; last && last->next; last = last->next)
		;

	if (parsedomains(s, '|', &d) < 0)
		return -1;
	if (last)
		last->next = d;
	else
		f->domains = d;

	return (d != NULL);
}

static int
filtertype_cmp(const void *a, const void *b)
{
	return strcmp(((struct filtertype *)a)->name,
	              ((struct filtertype *)b)->name);
}

/* check if domain is the same domain or a subdomain of `s` */
static int
matchdomain(const char *s, const char *domain)
{
	size_t l1, l2;

	l1 = strlen(s);
	l2 = strlen(domain);

	/* subdomain-specific (longer) or other domain */
	if (l1 > l2)
		return 0;
	/* subdomain */
	if (l2 > l1 && domain[l2 - l1 - 1] == '.')
		return !strcmp(&domain[l2 - l1], s);

	return !strcmp(s, domain);
}

static int
matchrule(struct filterrule *f, const char *fromuri, const char *fromdomain,
	const char *fromrel,
	const char *requri, const char *reqdomain, const char *reqrel,
	const char *type)
{
	/* NOTE: order matters, see FilterType enum values */
	struct filterdomain *d;
	char pat[1024];
	const char *uri;
	int len, r;

	r = f->domains ? 0 : 1;
	for (d = f->domains; d; d = d->next) {
		if (matchdomain(d->domain, fromdomain)) {
			if (r && d->inverse)
				r = 0;
			else if (!r && !d->inverse)
				r = 1;
		} else if (r && !d->inverse) {
			r = 0;
		}
	}
	if (f->css) {
		/* DEBUG */
#if 0
		if (f->isexception)
			printf("DEBUG, exception rule, CSS: %s, match? %d\n",
			f->css, r);
#endif
		return r;
	}

#if 1
	/* skip allow rule, TODO: inverse? */
	if (!r)
		return 0;
#endif

	/* match begin including domain */
	if (f->matchbegin) {
		/* TODO: match domain part of pattern */
		/* TODO: preprocess pattern if it is matchbegin? */

		len = strcspn(f->uri, "^/");

		/* match domain without dot */
		r = snprintf(pat, sizeof(pat), "%.*s",
			len, f->uri);
		if (r == -1 || (size_t)r >= sizeof(pat)) {
			fprintf(stderr, "warning: pattern too large, ignoring\n");
			return 0;
		}

		/* TODO: block type mask */
		if (match(pat, reqdomain, (f->block & FilterTypeMatchCase) ? 0 : 1)) {
			/* match domain with dot */
			r = snprintf(pat, sizeof(pat), "*.%.*s",
				len, f->uri);
			if (r == -1 || (size_t)r >= sizeof(pat)) {
				fprintf(stderr, "warning: pattern too large, ignoring\n");
				return 0;
			}

			/* TODO: block type mask */
			if (match(pat, reqdomain, (f->block & FilterTypeMatchCase) ? 0 : 1))
				return 0;
		}

		/* match on path */
		r = snprintf(pat, sizeof(pat), "*%s%s",
			f->uri + len,
			f->matchend ? "" : "*");
		uri = reqrel;
	} else {
		r = snprintf(pat, sizeof(pat), "*%s%s",
			f->uri,
			f->matchend ? "" : "*");
		uri = requri;

	}
	if (r == -1 || (size_t)r >= sizeof(pat)) {
		fprintf(stderr, "warning: pattern too large, ignoring\n");
		return 0;
	}

	/* TODO: block type mask */
	if (!match(pat, uri, (f->block & FilterTypeMatchCase) ? 0 : 1))
		return 1;

	return 0;
}

static int
parserule(struct filterrule *f, char *s)
{
	struct filtertype key, *ft;
	int inverse = 0;
	char *p, *values;

	if (*s == '!' || (*s == '[' && s[strlen(s) - 1] == ']'))
		return 0; /* skip comment or empty line */
	for (; *s && isspace(*s); s++)
		;
	if (!*s)
		return 0; /* line had only whitespace: skip */

	memset(f, 0, sizeof(struct filterrule));

	if ((p = strstr(s, "#@#"))) {
		*p = '\0';
		if (parsedomainselement(f, s) < 0)
			return -1;
		*p = '#';
		if (!(f->css = westrdup(p + 3)))
			return -1;
		f->isexception = 1;
		goto end; /* end of CSS rule */
	}

	/* element hiding rule, NOTE: no wildcards are supported,
	   "Simplified element hiding syntax" (legacy) is not supported. */
	if ((p = strstr(s, "##"))) {
		*p = '\0';
		if (parsedomainselement(f, s) < 0)
			return -1;
		*p = '#';
		if (!(f->css = westrdup(p + 2)))
			return -1;
		goto end; /* end of rule */
	}

	if (!strncmp(s, "@@", 2)) {
		f->isexception = 1;
		s += 2;
	}
	if (*s == '|') {
		s++;
		if (*s == '|') {
			f->matchbegin = 1;
			s++;
		} else {
			f->matchend = 1;
		}
	}

	/* no options, use rest of line as uri. */
	if (!(p = strrchr(s, '$'))) {
		if (!(f->uri = westrdup(s)))
			return -1;
		goto end;
	}

	/* has options */
	if (!(f->uri = westrndup(s, p - s)))
		return -1;

	s = ++p;

	/* blockmask, has options? default: allow all options, case-sensitive
	 * has no options? default: block all options, case-sensitive  */
	f->block = *s ? (unsigned long)FilterTypeMatchCase : ~0UL;
	do {
		if ((p = strchr(s, ',')))
			*p = '\0';
		/* match option */
		inverse = 0;
		if (*s == '~') {
			inverse = 1;
			s++;
		}
		if ((values = strchr(s, '=')))
			*(values) = '\0';
		key.name = s;

		ft = bsearch(&key, &filtertypes,
		             sizeof(filtertypes) / sizeof(*filtertypes),
		             sizeof(*filtertypes), filtertype_cmp);

		/* restore NUL-terminator for domain= option */
		if (values)
			*(values++) = '=';

		if (ft) {
			if (inverse)
				f->block &= ~(ft->type);
			else
				f->block |= ft->type;
			if (ft->fn && values)
				ft->fn(f, values);
		} else {
			/* DEBUG */
#if 0
			fprintf(stderr, "ignored: unknown option: '%s' "
			        "in rule: %s\n", key.name, f->uri);
#endif
		}

		/* restore ',' */
		if (p) {
			*p = ',';
			s = p + 1;
		}
	} while (p);
end:

	return 1;
}

#if 0
static void
debugrule(struct filterrule *r)
{
	printf("\turi: %s\n\tcss: %s\n\tisexception: %d\n\tblockmask: "
	       "%lu\n===\n", r->uri ? r->uri : "", r->css ? r->css : "",
	       r->isexception, r->block);
}
#endif

static int
loadrules(FILE *fp)
{
	struct filterrule f, *r, *rn = NULL;
	char *line = NULL;
	size_t linesiz = 0;
	ssize_t n;
	int ret;

	/* load rules */
	while ((n = getline(&line, &linesiz, fp)) > 0) {
		if (line[n - 1] == '\n')
			line[--n] = '\0';
		if (n > 0 && line[n - 1] == '\r')
			line[--n] = '\0';

		if ((ret = parserule(&f, line) > 0)) {
			if (!(r = wecalloc(1, sizeof(struct filterrule))))
				return -1;
			if (!rules)
				rules = rn = r;
			else
				rn = rn->next = r;
			memcpy(rn, &f, sizeof(struct filterrule));
		} else if (ret < 0) {
			return -1;
		}
	}
	if (ferror(fp)) {
		weprintf("getline: %s\n", strerror(errno));
		return -1;
	}
	return (rules != NULL);
}

char *
getglobalcss(void)
{
	return globalcss.data;
}

char *
getdocumentcss(const char *fromuri)
{
	const char *s;
	char fromdomain[256];
	String sitecss;
	struct filterrule *r;
	size_t len;

	/* skip protocol */
	if ((s = strstr(fromuri, "://")))
		fromuri = s + sizeof("://") - 1;
	len = strcspn(fromuri, "/"); /* TODO: ":/" */
	memcpy(fromdomain, fromuri, len);
	fromdomain[len] = '\0';

	printf("fromuri:    %s\n", fromuri);
	printf("fromdomain: %s\n", fromdomain);

	/* DEBUG: timing */
	struct timespec tp_start, tp_end, tp_diff;
	if (clock_gettime(CLOCK_MONOTONIC, &tp_start) == -1) {
		fprintf(stderr, "clock_gettime: %s\n", strerror(errno));
	}

	/* site-specific CSS */
	memset(&sitecss, 0, sizeof(sitecss));
	for (r = rules; r; r = r->next) {
		if (!r->css || !r->domains ||
		    !matchrule(r, "", fromdomain, "", "", "", "", ""))
			continue;

		len = strlen(r->css);
		if (string_append(&sitecss, r->css, len) < len)
			goto err;

		s = r->isexception ? "{display:initial;}" : "{display:none;}";
		len = strlen(s);
		if (string_append(&sitecss, s, len) < len)
			goto err;
	}
/*	printf("sitecss: %s\n", sitecss.data ? sitecss.data : "<empty>");*/

	/* DEBUG: timing */
	if (clock_gettime(CLOCK_MONOTONIC, &tp_end) == -1) {
		fprintf(stderr, "clock_gettime: %s\n", strerror(errno));
	}

	tp_diff.tv_sec = tp_end.tv_sec - tp_start.tv_sec;
	tp_diff.tv_nsec = tp_end.tv_nsec - tp_start.tv_nsec;
	if (tp_diff.tv_nsec < 0) {
		tp_diff.tv_sec--;
		tp_diff.tv_nsec += 1000000000L;
	}

	printf("timing: %lld sec, %.3f ms\n",
		(long long)tp_diff.tv_sec, (float)tp_diff.tv_nsec / 1000000.0f);

	if (globalcss.data)
		printf("global CSS length in bytes: %zu\n", strlen(globalcss.data));
	if (sitecss.data)
		printf("site CSS length in bytes: %zu\n", strlen(sitecss.data));

	return sitecss.data;

err:
	free(sitecss.data);
	/*memset(&sitecss, 0, sizeof(sitecss));*/

	return NULL;
}

int
allowrequest(const char *fromuri, const char *requri)
{
	struct filterrule *r;
	char fromdomain[256], reqdomain[256];
	const char *s, *reqrel, *fromrel;
	size_t len;
	int status = 1;

	/* skip protocol part */
	if ((s = strstr(fromuri, "://")))
		fromuri = s + sizeof("://") - 1;
	if ((s = strstr(requri, "://")))
		requri = s + sizeof("://") - 1;

	len = strcspn(fromuri, ":/"); /* TODO: ":/", but support IPV6... */
	memcpy(fromdomain, fromuri, len);
	fromdomain[len] = '\0';

	len = strcspn(requri, ":/"); /* TODO: ":/", but support IPV6... */
	memcpy(reqdomain, requri, len);
	reqdomain[len] = '\0';

	fromrel = &fromuri[strcspn(fromuri, "/")];
	reqrel = &requri[strcspn(requri, "/")];

#if 0
	printf("req %s = %s\n", requri, reqrel);
	printf("from %s = %s\n", fromuri, fromrel);
#endif

	/* DEBUG: timing */
	struct timespec tp_start, tp_end, tp_diff;
	if (clock_gettime(CLOCK_MONOTONIC, &tp_start) == -1)
		fprintf(stderr, "clock_gettime: %s\n", strerror(errno));

	/* match rules */
	for (r = rules; r; r = r->next) {
		if (!r->css && matchrule(r, fromuri, fromdomain,
		                         fromrel, requri, reqdomain, reqrel, "csio^")) {
#if 0
			printf("reqrel:      %s\n", reqrel);
			printf("reqdomain:   %s\n", reqdomain);
			printf("requri:      %s\n", requri);
			printf("from uri:    %s\n", fromuri);
			printf("from domain: %s\n", fromdomain);
#endif

			fprintf(stderr, "blocked: %s, %s\n", fromdomain, requri);
			fprintf(stderr, "rule:    %s\n", r->uri);
			fprintf(stderr, "===\n");

			/* DEBUG: for showing the timing */
			status = 0;
			goto end;
			/*return 1;*/
		}
	}

end:
	/* DEBUG: timing */
	if (clock_gettime(CLOCK_MONOTONIC, &tp_end) == -1) {
		fprintf(stderr, "clock_gettime: %s\n", strerror(errno));
	}

	tp_diff.tv_sec = tp_end.tv_sec - tp_start.tv_sec;
	tp_diff.tv_nsec = tp_end.tv_nsec - tp_start.tv_nsec;
	if (tp_diff.tv_nsec < 0) {
		tp_diff.tv_sec--;
		tp_diff.tv_nsec += 1000000000L;
	}

	printf("%s [%s] timing: %lld sec, %.3f ms\n",
		requri, fromuri, (long long)tp_diff.tv_sec,
		(float)tp_diff.tv_nsec / 1000000.0f);

	return status;
}

void
cleanup(void)
{
	struct filterrule *r;
	struct filterdomain *d;

	free(globalcss.data);
	memset(&globalcss, 0, sizeof(globalcss));

	for (r = rules; r; r = rules) {
		for (d = r->domains; d; d = r->domains) {
			free(d->domain);
			r->domains = d->next;
			free(d);
		}
		free(r->css);
		free(r->uri);
		rules = r->next;
		free(r);
	}
	rules = NULL;
}

void
init(void)
{
	struct filterrule *r;
	FILE *fp;
	const char *s;
	char filepath[PATH_MAX], *e;
	size_t len;
	int n;

	if ((e = getenv("SURF_ADBLOCK_FILE"))) {
		n = snprintf(filepath, sizeof(filepath), "%s", e);
	} else {
		if (!(e = getenv("HOME")))
			e = "";
		n = snprintf(filepath, sizeof(filepath),
		             "%s%s.surf/adblockrules", e, e[0] ? "/" : "");
	}
	if (n < 0 || (size_t)n >= sizeof(filepath)) {
		weprintf("fatal: rules file path too long");
		return;
	}

	if (!(fp = fopen(filepath, "r"))) {
		weprintf("fatal: cannot open rules file %s: %s\n",
		         filepath, strerror(errno));
		return;
	}

	n = loadrules(fp);
	fclose(fp);
	if (n < 1) {
		if (n < 0) {
			weprintf("fatal: cannot read rules from file %s: %s\n",
			         filepath, strerror(errno));
		} else  {
			weprintf("fatal: cannot read any rule from file %s\n",
			         filepath);
		}
		return;
	}

	/* general CSS rules: all sites */
	for (r = rules; r; r = r->next) {
		if (!r->css || r->domains)
			continue;

		len = strlen(r->css);
		if (string_append(&globalcss, r->css, len) < len) {
			weprintf("cannot append CSS rule to global CSS selectors\n");
			cleanup();
			return;
		}

		s = r->isexception ? "{display:initial;}" : "{display:none;}";
		len = strlen(s);
		if (string_append(&globalcss, s, len) < len) {
			weprintf("cannot append CSS rule to global CSS selectors\n");
			cleanup();
			return;
		}
	}
}
