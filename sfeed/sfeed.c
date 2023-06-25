#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "util.h"
#include "xml.h"

#define ISINCONTENT(ctx)  ((ctx).iscontent && !((ctx).iscontenttag))
#define ISCONTENTTAG(ctx) (!((ctx).iscontent) && (ctx).iscontenttag)

/* these feed fields support multiple separated values */
#define ISFEEDFIELDMULTI(t) ((t) == FeedFieldCategory)

/* string and byte-length */
#define STRP(s)           s,sizeof(s)-1

enum FeedType {
	FeedTypeNone = 0,
	FeedTypeRSS  = 1,
	FeedTypeAtom = 2
};

enum ContentType {
	ContentTypeNone  = 0,
	ContentTypePlain = 1,
	ContentTypeHTML  = 2
};
static const char *contenttypes[] = { "", "plain", "html" };

/* String data / memory pool */
typedef struct string {
	char   *data;   /* data */
	size_t  len;    /* string length */
	size_t  bufsiz; /* allocated size */
} String;

/* NOTE: the order of these fields (content, date, author) indicate the
 *       priority to use them, from least important to high. */
enum TagId {
	TagUnknown = 0,
	/* RSS */
	RSSTagDcdate, RSSTagPubdate, /* creation date has higher priority */
	RSSTagTitle,
	RSSTagMediaDescription, RSSTagDescription, RSSTagContentEncoded,
	RSSTagGuid,
	RSSTagGuidPermalinkFalse,
	RSSTagGuidPermalinkTrue,
	/* must be defined after GUID, because it can be a link (isPermaLink) */
	RSSTagLink,
	RSSTagEnclosure,
	RSSTagAuthor, RSSTagDccreator,
	RSSTagCategory,
	/* Atom */
	/* creation date has higher priority */
	AtomTagModified, AtomTagUpdated, AtomTagIssued, AtomTagPublished,
	AtomTagTitle,
	AtomTagMediaDescription, AtomTagSummary, AtomTagContent,
	AtomTagId,
	AtomTagLink,
	AtomTagLinkAlternate,
	AtomTagLinkEnclosure,
	AtomTagAuthor, AtomTagAuthorName,
	AtomTagCategory,
	TagLast
};

typedef struct feedtag {
	char       *name; /* name of tag to match */
	size_t      len;  /* len of `name` */
	enum TagId  id;   /* unique ID */
} FeedTag;

typedef struct field {
	String     str;
	enum TagId tagid; /* tagid set previously, used for tag priority */
} FeedField;

enum {
	FeedFieldTime = 0, FeedFieldTitle, FeedFieldLink, FeedFieldContent,
	FeedFieldId, FeedFieldAuthor, FeedFieldEnclosure, FeedFieldCategory,
	FeedFieldLast
};

typedef struct feedcontext {
	String          *field;        /* current FeedItem field String */
	FeedField        fields[FeedFieldLast]; /* data for current item */
	FeedTag          tag;          /* unique current parsed tag */
	int              iscontent;    /* in content data */
	int              iscontenttag; /* in content tag */
	enum ContentType contenttype;  /* content-type for item */
	enum FeedType    feedtype;
	int              attrcount;    /* count item HTML element attributes */
} FeedContext;

static long long datetounix(long long, int, int, int, int, int);
static FeedTag * gettag(enum FeedType, const char *, size_t);
static long gettzoffset(const char *);
static int  isattr(const char *, size_t, const char *, size_t);
static int  istag(const char *, size_t, const char *, size_t);
static int  parsetime(const char *, long long *);
static void printfields(void);
static void string_append(String *, const char *, size_t);
static void string_buffer_realloc(String *, size_t);
static void string_clear(String *);
static void string_print_encoded(String *);
static void string_print_timestamp(String *);
static void string_print_trimmed(String *);
static void string_print_trimmed_multi(String *);
static void string_print_uri(String *);
static void xmlattr(XMLParser *, const char *, size_t, const char *, size_t,
                    const char *, size_t);
static void xmlattrentity(XMLParser *, const char *, size_t, const char *,
                          size_t, const char *, size_t);
static void xmlattrend(XMLParser *, const char *, size_t, const char *,
                       size_t);
static void xmlattrstart(XMLParser *, const char *, size_t, const char *,
                         size_t);
static void xmldata(XMLParser *, const char *, size_t);
static void xmldataentity(XMLParser *, const char *, size_t);
static void xmltagend(XMLParser *, const char *, size_t, int);
static void xmltagstart(XMLParser *, const char *, size_t);
static void xmltagstartparsed(XMLParser *, const char *, size_t, int);

/* map tag name to TagId type */
/* RSS, must be alphabetical order */
static const FeedTag rsstags[] = {
	{ STRP("author"),            RSSTagAuthor            },
	{ STRP("category"),          RSSTagCategory          },
	{ STRP("content:encoded"),   RSSTagContentEncoded    },
	{ STRP("dc:creator"),        RSSTagDccreator         },
	{ STRP("dc:date"),           RSSTagDcdate            },
	{ STRP("description"),       RSSTagDescription       },
	/* RSS: <enclosure url="" />, Atom has <link rel="enclosure" /> */
	{ STRP("enclosure"),         RSSTagEnclosure         },
	{ STRP("guid"),              RSSTagGuid              },
	{ STRP("link"),              RSSTagLink              },
	{ STRP("media:description"), RSSTagMediaDescription  },
	{ STRP("pubdate"),           RSSTagPubdate           },
	{ STRP("title"),             RSSTagTitle             }
};

/* Atom, must be alphabetical order */
static const FeedTag atomtags[] = {
	{ STRP("author"),            AtomTagAuthor           },
	{ STRP("category"),          AtomTagCategory         },
	{ STRP("content"),           AtomTagContent          },
	{ STRP("id"),                AtomTagId               },
	{ STRP("issued"),            AtomTagIssued           }, /* Atom 0.3 */
	/* Atom: <link href="" />, RSS has <link></link> */
	{ STRP("link"),              AtomTagLink             },
	{ STRP("media:description"), AtomTagMediaDescription },
	{ STRP("modified"),          AtomTagModified         }, /* Atom 0.3 */
	{ STRP("published"),         AtomTagPublished        },
	{ STRP("summary"),           AtomTagSummary          },
	{ STRP("title"),             AtomTagTitle            },
	{ STRP("updated"),           AtomTagUpdated          }
};

/* special case: nested <author><name> */
static const FeedTag atomtagauthor = { STRP("author"), AtomTagAuthor };
static const FeedTag atomtagauthorname = { STRP("name"), AtomTagAuthorName };

/* reference to no / unknown tag */
static const FeedTag notag = { STRP(""), TagUnknown };

/* map TagId type to RSS/Atom field, all tags must be defined */
static const int fieldmap[TagLast] = {
	[TagUnknown]               = -1,
	/* RSS */
	[RSSTagDcdate]             = FeedFieldTime,
	[RSSTagPubdate]            = FeedFieldTime,
	[RSSTagTitle]              = FeedFieldTitle,
	[RSSTagMediaDescription]   = FeedFieldContent,
	[RSSTagDescription]        = FeedFieldContent,
	[RSSTagContentEncoded]     = FeedFieldContent,
	[RSSTagGuid]               = -1,
	[RSSTagGuidPermalinkFalse] = FeedFieldId,
	[RSSTagGuidPermalinkTrue]  = FeedFieldId, /* special-case: both a link and an id */
	[RSSTagLink]               = FeedFieldLink,
	[RSSTagEnclosure]          = FeedFieldEnclosure,
	[RSSTagAuthor]             = FeedFieldAuthor,
	[RSSTagDccreator]          = FeedFieldAuthor,
	[RSSTagCategory]           = FeedFieldCategory,
	/* Atom */
	[AtomTagModified]          = FeedFieldTime,
	[AtomTagUpdated]           = FeedFieldTime,
	[AtomTagIssued]            = FeedFieldTime,
	[AtomTagPublished]         = FeedFieldTime,
	[AtomTagTitle]             = FeedFieldTitle,
	[AtomTagMediaDescription]  = FeedFieldContent,
	[AtomTagSummary]           = FeedFieldContent,
	[AtomTagContent]           = FeedFieldContent,
	[AtomTagId]                = FeedFieldId,
	[AtomTagLink]              = -1,
	[AtomTagLinkAlternate]     = FeedFieldLink,
	[AtomTagLinkEnclosure]     = FeedFieldEnclosure,
	[AtomTagAuthor]            = -1,
	[AtomTagAuthorName]        = FeedFieldAuthor,
	[AtomTagCategory]          = FeedFieldCategory
};

static const int FieldSeparator = '\t';
/* separator for multiple values in a field, separator should be 1 byte */
static const char FieldMultiSeparator[] = "|";
static struct uri baseuri;
static const char *baseurl;

static FeedContext ctx;
static XMLParser parser; /* XML parser state */
static String attrispermalink, attrrel, attrtype, tmpstr;

static int
tagcmp(const void *v1, const void *v2)
{
	return strcasecmp(((FeedTag *)v1)->name, ((FeedTag *)v2)->name);
}

/* Unique tagid for parsed tag name. */
static FeedTag *
gettag(enum FeedType feedtype, const char *name, size_t namelen)
{
	FeedTag f, *r = NULL;

	f.name = (char *)name;

	switch (feedtype) {
	case FeedTypeRSS:
		r = bsearch(&f, rsstags, sizeof(rsstags) / sizeof(rsstags[0]),
		        sizeof(rsstags[0]), tagcmp);
		break;
	case FeedTypeAtom:
		r = bsearch(&f, atomtags, sizeof(atomtags) / sizeof(atomtags[0]),
		        sizeof(atomtags[0]), tagcmp);
		break;
	default:
		break;
	}

	return r;
}

static char *
ltrim(const char *s)
{
	for (; ISSPACE((unsigned char)*s); s++)
		;
	return (char *)s;
}

static char *
rtrim(const char *s)
{
	const char *e;

	for (e = s + strlen(s); e > s && ISSPACE((unsigned char)*(e - 1)); e--)
		;
	return (char *)e;
}

/* Clear string only; don't free, prevents unnecessary reallocation. */
static void
string_clear(String *s)
{
	if (s->data)
		s->data[0] = '\0';
	s->len = 0;
}

static void
string_buffer_realloc(String *s, size_t newlen)
{
	size_t alloclen;

	if (newlen > SIZE_MAX / 2) {
		alloclen = SIZE_MAX;
	} else {
		for (alloclen = 64; alloclen <= newlen; alloclen *= 2)
			;
	}
	if (!(s->data = realloc(s->data, alloclen)))
		err(1, "realloc");
	s->bufsiz = alloclen;
}

/* Append data to String, s->data and data may not overlap. */
static void
string_append(String *s, const char *data, size_t len)
{
	if (!len)
		return;

	if (s->len >= SIZE_MAX - len) {
		errno = ENOMEM;
		err(1, "realloc");
	}

	/* check if allocation is necessary, never shrink the buffer. */
	if (s->len + len >= s->bufsiz)
		string_buffer_realloc(s, s->len + len + 1);
	memcpy(s->data + s->len, data, len);
	s->len += len;
	s->data[s->len] = '\0';
}

/* Print text, encode TABs, newlines and '\', remove other whitespace.
 * Remove leading and trailing whitespace. */
static void
string_print_encoded(String *s)
{
	const char *p, *e;

	if (!s->data || !s->len)
		return;

	p = ltrim(s->data);
	e = rtrim(p);

	for (; *p && p != e; p++) {
		switch (*p) {
		case '\n': putchar('\\'); putchar('n'); break;
		case '\\': putchar('\\'); putchar('\\'); break;
		case '\t': putchar('\\'); putchar('t'); break;
		default:
			/* ignore control chars */
			if (!ISCNTRL((unsigned char)*p))
				putchar(*p);
			break;
		}
	}
}

static void
printtrimmed(const char *s)
{
	char *p, *e;

	p = ltrim(s);
	e = rtrim(p);
	for (; *p && p != e; p++) {
		if (ISSPACE((unsigned char)*p))
			putchar(' '); /* any whitespace to space */
		else if (!ISCNTRL((unsigned char)*p))
			/* ignore other control chars */
			putchar(*p);
	}
}

/* Print text, replace TABs, carriage return and other whitespace with ' '.
 * Other control chars are removed. Remove leading and trailing whitespace. */
static void
string_print_trimmed(String *s)
{
	if (!s->data || !s->len)
		return;

	printtrimmed(s->data);
}

/* Print each field with trimmed whitespace, separated by '|'. */
static void
string_print_trimmed_multi(String *s)
{
	char *p, *e;
	int c;

	if (!s->data || !s->len)
		return;

	for (p = s->data; ; p = e + 1) {
		if ((e = strstr(p, FieldMultiSeparator))) {
			c = *e;
			*e = '\0';
			printtrimmed(p);
			*e = c; /* restore NUL byte to original character */
			fputs(FieldMultiSeparator, stdout);
		} else {
			printtrimmed(p);
			break;
		}
	}
}

/* Print URL, if it is a relative URL then it uses the global `baseurl`. */
static void
printuri(char *s)
{
	char link[4096], *p, *e;
	struct uri newuri, olduri;
	int c, r = -1;

	p = ltrim(s);
	e = rtrim(p);
	c = *e;
	*e = '\0';

	if (baseurl && !uri_hasscheme(p) &&
	    uri_parse(p, &olduri) != -1 && !olduri.proto[0] &&
	    uri_makeabs(&newuri, &olduri, &baseuri) != -1 && newuri.proto[0])
		r = uri_format(link, sizeof(link), &newuri);

	if (r >= 0 && (size_t)r < sizeof(link))
		printtrimmed(link);
	else
		printtrimmed(p);

	*e = c; /* restore NUL byte to original character */
}

/* Print URL, if it is a relative URL then it uses the global `baseurl`. */
static void
string_print_uri(String *s)
{
	if (!s->data || !s->len)
		return;

	printuri(s->data);
}

/* Print as UNIX timestamp, print nothing if the time is empty or invalid. */
static void
string_print_timestamp(String *s)
{
	long long t;

	if (!s->data || !s->len)
		return;

	if (parsetime(s->data, &t) != -1)
		printf("%lld", t);
}

/* Convert time fields. Returns a signed (at least) 64-bit UNIX timestamp.
   Parameters should be passed as they are in a struct tm:
   that is: year = year - 1900, month = month - 1. */
static long long
datetounix(long long year, int mon, int day, int hour, int min, int sec)
{
	/* seconds in a month in a regular (non-leap) year */
	static const long secs_through_month[] = {
		0, 31 * 86400, 59 * 86400, 90 * 86400,
		120 * 86400, 151 * 86400, 181 * 86400, 212 * 86400,
		243 * 86400, 273 * 86400, 304 * 86400, 334 * 86400 };
	int is_leap = 0, cycles, centuries = 0, leaps = 0, rem;
	long long t;

	/* optimization: handle common range year 1902 up to and including 2038 */
	if (year - 2ULL <= 136) {
		/* amount of leap days relative to 1970: every 4 years */
		leaps = (year - 68) >> 2;
		if (!((year - 68) & 3)) {
			leaps--;
			is_leap = 1;
		} else {
			is_leap = 0;
		}
		t = 31536000 * (year - 70) + (86400 * leaps); /* 365 * 86400 = 31536000 */
	} else {
		/* general leap year calculation:
		   leap years occur mostly every 4 years but every 100 years
		   a leap year is skipped unless the year is divisible by 400 */
		cycles = (year - 100) / 400;
		rem = (year - 100) % 400;
		if (rem < 0) {
			cycles--;
			rem += 400;
		}
		if (!rem) {
			is_leap = 1;
		} else {
			if (rem >= 300) {
				centuries = 3;
				rem -= 300;
			} else if (rem >= 200) {
				centuries = 2;
				rem -= 200;
			} else if (rem >= 100) {
				centuries = 1;
				rem -= 100;
			}
			if (rem) {
				leaps = rem / 4U;
				rem %= 4U;
				is_leap = !rem;
			}
		}
		leaps += (97 * cycles) + (24 * centuries) - is_leap;

		/* adjust 8 leap days from 1970 up to and including 2000:
		   ((30 * 365) + 8) * 86400 = 946771200 */
		t = ((year - 100) * 31536000LL) + (leaps * 86400LL) + 946771200LL;
	}
	t += secs_through_month[mon];
	if (is_leap && mon >= 2)
		t += 86400;
	t += 86400LL * (day - 1);
	t += 3600LL * hour;
	t += 60LL * min;
	t += sec;

	return t;
}

/* Get timezone from string, return time offset in seconds from UTC.
 * NOTE: only parses timezones in RFC 822, many other timezone names are
 * ambiguous anyway.
 * ANSI and military zones are defined wrong in RFC 822 and are unsupported,
 * see note on RFC 2822 4.3 page 32. */
static long
gettzoffset(const char *s)
{
	static const struct {
		char *name;
		int offhour;
	} tzones[] = {
		{ "CDT", -5 * 3600 },
		{ "CST", -6 * 3600 },
		{ "EDT", -4 * 3600 },
		{ "EST", -5 * 3600 },
		{ "MDT", -6 * 3600 },
		{ "MST", -7 * 3600 },
		{ "PDT", -7 * 3600 },
		{ "PST", -8 * 3600 },
	};
	const char *p;
	long tzhour = 0, tzmin = 0;
	size_t i;

	for (; ISSPACE((unsigned char)*s); s++)
		;
	switch (*s) {
	case '-': /* offset */
	case '+':
		for (i = 0, p = s + 1; i < 2 && ISDIGIT((unsigned char)*p); i++, p++)
			tzhour = (tzhour * 10) + (*p - '0');
		if (*p == ':')
			p++;
		for (i = 0; i < 2 && ISDIGIT((unsigned char)*p); i++, p++)
			tzmin = (tzmin * 10) + (*p - '0');
		return ((tzhour * 3600) + (tzmin * 60)) * (s[0] == '-' ? -1 : 1);
	default: /* timezone name */
		for (i = 0; ISALPHA((unsigned char)s[i]); i++)
			;
		if (i != 3)
			return 0;
		/* compare timezone and adjust offset relative to UTC */
		for (i = 0; i < sizeof(tzones) / sizeof(*tzones); i++) {
			if (!memcmp(s, tzones[i].name, 3))
				return tzones[i].offhour;
		}
	}
	return 0;
}

/* Parse time string `s` into the UNIX timestamp `tp`.
   Returns 0 on success or -1 on failure. */
static int
parsetime(const char *s, long long *tp)
{
	static const struct {
		char *name;
		int len;
	} mons[] = {
		{ STRP("January"),   },
		{ STRP("February"),  },
		{ STRP("March"),     },
		{ STRP("April"),     },
		{ STRP("May"),       },
		{ STRP("June"),      },
		{ STRP("July"),      },
		{ STRP("August"),    },
		{ STRP("September"), },
		{ STRP("October"),   },
		{ STRP("November"),  },
		{ STRP("December"),  },
	};
	int va[6] = { 0 }, i, j, v, vi;
	size_t m;

	for (; ISSPACE((unsigned char)*s); s++)
		;
	if (!ISDIGIT((unsigned char)*s) && !ISALPHA((unsigned char)*s))
		return -1;

	if (ISDIGIT((unsigned char)s[0]) &&
	    ISDIGIT((unsigned char)s[1]) &&
	    ISDIGIT((unsigned char)s[2]) &&
	    ISDIGIT((unsigned char)s[3])) {
		/* formats "%Y-%m-%d %H:%M:%S", "%Y-%m-%dT%H:%M:%S" or "%Y%m%d%H%M%S" */
		vi = 0;
	} else {
		/* format: "[%a, ]%d %b %Y %H:%M:%S" */
		/* parse "[%a, ]%d %b %Y " part, then use time parsing as above */
		for (; ISALPHA((unsigned char)*s); s++)
			;
		for (; ISSPACE((unsigned char)*s); s++)
			;
		if (*s == ',')
			s++;
		for (; ISSPACE((unsigned char)*s); s++)
			;
		for (v = 0, i = 0; i < 2 && ISDIGIT((unsigned char)*s); s++, i++)
			v = (v * 10) + (*s - '0');
		va[2] = v; /* day */
		for (; ISSPACE((unsigned char)*s); s++)
			;
		/* end of word month */
		for (j = 0; ISALPHA((unsigned char)s[j]); j++)
			;
		/* check month name */
		if (j < 3 || j > 9)
			return -1; /* month cannot match */
		for (m = 0; m < sizeof(mons) / sizeof(*mons); m++) {
			/* abbreviation (3 length) or long name */
			if ((j == 3 || j == mons[m].len) &&
			    !strncasecmp(mons[m].name, s, j)) {
				va[1] = m + 1;
				s += j;
				break;
			}
		}
		if (m >= 12)
			return -1; /* no month found */
		for (; ISSPACE((unsigned char)*s); s++)
			;
		for (v = 0, i = 0; i < 4 && ISDIGIT((unsigned char)*s); s++, i++)
			v = (v * 10) + (*s - '0');
		/* obsolete short year: RFC 2822 4.3 */
		if (i == 2 || i == 3)
			v += (i == 2 && v >= 0 && v <= 49) ? 2000 : 1900;
		va[0] = v; /* year */
		for (; ISSPACE((unsigned char)*s); s++)
			;
		/* parse only regular time part, see below */
		vi = 3;
	}

	/* parse time parts (and possibly remaining date parts) */
	for (; *s && vi < 6; vi++) {
		for (i = 0, v = 0; i < ((vi == 0) ? 4 : 2) &&
		                   ISDIGIT((unsigned char)*s); s++, i++) {
			v = (v * 10) + (*s - '0');
		}
		va[vi] = v;

		if ((vi < 2 && *s == '-') ||
		    (vi == 2 && (*s == 'T' || ISSPACE((unsigned char)*s))) ||
		    (vi > 2 && *s == ':'))
			s++;
	}

	/* skip milliseconds in for example: "%Y-%m-%dT%H:%M:%S.000Z" */
	if (*s == '.') {
		for (s++; ISDIGIT((unsigned char)*s); s++)
			;
	}

	/* invalid range */
	if (va[0] < 0 || va[0] > 9999 ||
	    va[1] < 1 || va[1] > 12 ||
	    va[2] < 1 || va[2] > 31 ||
	    va[3] < 0 || va[3] > 23 ||
	    va[4] < 0 || va[4] > 59 ||
	    va[5] < 0 || va[5] > 60) /* allow leap second */
		return -1;

	*tp = datetounix(va[0] - 1900, va[1] - 1, va[2], va[3], va[4], va[5]) -
	      gettzoffset(s);

	return 0;
}

static void
printfields(void)
{
	string_print_timestamp(&ctx.fields[FeedFieldTime].str);
	putchar(FieldSeparator);
	string_print_trimmed(&ctx.fields[FeedFieldTitle].str);
	putchar(FieldSeparator);
	string_print_uri(&ctx.fields[FeedFieldLink].str);
	putchar(FieldSeparator);
	string_print_encoded(&ctx.fields[FeedFieldContent].str);
	putchar(FieldSeparator);
	fputs(contenttypes[ctx.contenttype], stdout);
	putchar(FieldSeparator);
	string_print_trimmed(&ctx.fields[FeedFieldId].str);
	putchar(FieldSeparator);
	string_print_trimmed(&ctx.fields[FeedFieldAuthor].str);
	putchar(FieldSeparator);
	string_print_uri(&ctx.fields[FeedFieldEnclosure].str);
	putchar(FieldSeparator);
	string_print_trimmed_multi(&ctx.fields[FeedFieldCategory].str);
	putchar('\n');

	if (ferror(stdout)) /* check for errors but do not flush */
		checkfileerror(stdout, "<stdout>", 'w');
}

static int
istag(const char *name, size_t len, const char *name2, size_t len2)
{
	return (len == len2 && !strcasecmp(name, name2));
}

static int
isattr(const char *name, size_t len, const char *name2, size_t len2)
{
	return (len == len2 && !strcasecmp(name, name2));
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	/* handles transforming inline XML to data */
	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML)
			xmldata(p, v, vl);
		return;
	}

	if (!ctx.tag.id)
		return;

	/* content-type may be: Atom: text, xhtml, html or mime-type.
	   MRSS (media:description): plain, html. */
	if (ISCONTENTTAG(ctx)) {
		if (isattr(n, nl, STRP("type")))
			string_append(&attrtype, v, vl);
		return;
	}

	if (ctx.feedtype == FeedTypeRSS) {
		if (ctx.tag.id == RSSTagEnclosure &&
		    isattr(n, nl, STRP("url"))) {
			string_append(&tmpstr, v, vl);
		} else if (ctx.tag.id == RSSTagGuid &&
		           isattr(n, nl, STRP("ispermalink"))) {
			string_append(&attrispermalink, v, vl);
		}
	} else if (ctx.feedtype == FeedTypeAtom) {
		if (ctx.tag.id == AtomTagLink) {
			if (isattr(n, nl, STRP("rel"))) {
				string_append(&attrrel, v, vl);
			} else if (isattr(n, nl, STRP("href"))) {
				string_append(&tmpstr, v, vl);
			}
		} else if (ctx.tag.id == AtomTagCategory &&
			   isattr(n, nl, STRP("term"))) {
			string_append(&tmpstr, v, vl);
		}
	}
}

static void
xmlattrentity(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
              const char *data, size_t datalen)
{
	char buf[8];
	int len;

	/* handles transforming inline XML to data */
	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML)
			xmldata(p, data, datalen);
		return;
	}

	if (!ctx.tag.id)
		return;

	/* try to translate entity, else just pass as data to
	 * xmlattr handler. */
	if ((len = xml_entitytostr(data, buf, sizeof(buf))) > 0)
		xmlattr(p, t, tl, n, nl, buf, (size_t)len);
	else
		xmlattr(p, t, tl, n, nl, data, datalen);
}

static void
xmlattrend(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl)
{
	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML) {
			/* handles transforming inline XML to data */
			xmldata(p, "\"", 1);
			ctx.attrcount = 0;
		}
		return;
	}
}

static void
xmlattrstart(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl)
{
	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML) {
			/* handles transforming inline XML to data */
			if (!ctx.attrcount)
				xmldata(p, " ", 1);
			ctx.attrcount++;
			xmldata(p, n, nl);
			xmldata(p, "=\"", 2);
		}
		return;
	}

	if (attrispermalink.len && isattr(n, nl, STRP("ispermalink")))
		string_clear(&attrispermalink);
	else if (attrrel.len && isattr(n, nl, STRP("rel")))
		string_clear(&attrrel);
	else if (attrtype.len && isattr(n, nl, STRP("type")))
		string_clear(&attrtype);
	else if (tmpstr.len &&
	    (isattr(n, nl, STRP("href")) ||
	     isattr(n, nl, STRP("term")) ||
	     isattr(n, nl, STRP("url"))))
		string_clear(&tmpstr); /* use the last value for multiple attribute values */
}

static void
xmldata(XMLParser *p, const char *s, size_t len)
{
	if (!ctx.field)
		return;

	if (ISFEEDFIELDMULTI(fieldmap[ctx.tag.id]))
		string_append(&tmpstr, s, len);
	else
		string_append(ctx.field, s, len);
}

static void
xmldataentity(XMLParser *p, const char *data, size_t datalen)
{
	char buf[8];
	int len;

	if (!ctx.field)
		return;

	/* try to translate entity, else just pass as data to
	 * xmldata handler. */
	if ((len = xml_entitytostr(data, buf, sizeof(buf))) > 0)
		xmldata(p, buf, (size_t)len);
	else
		xmldata(p, data, datalen);
}

static void
xmltagstart(XMLParser *p, const char *t, size_t tl)
{
	const FeedTag *f;

	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML) {
			ctx.attrcount = 0;
			xmldata(p, "<", 1);
			xmldata(p, t, tl);
		}
		return;
	}

	/* start of RSS or Atom item / entry */
	if (ctx.feedtype == FeedTypeNone) {
		if (istag(t, tl, STRP("entry")))
			ctx.feedtype = FeedTypeAtom;
		else if (istag(t, tl, STRP("item")))
			ctx.feedtype = FeedTypeRSS;
		return;
	}

	/* field tagid already set or nested tags. */
	if (ctx.tag.id) {
		/* nested <author><name> for Atom */
		if (ctx.tag.id == AtomTagAuthor &&
		    istag(t, tl, STRP("name"))) {
			memcpy(&(ctx.tag), &atomtagauthorname, sizeof(ctx.tag));
		} else {
			return; /* other nested tags are not allowed: return */
		}
	}

	/* in item */
	if (ctx.tag.id == TagUnknown) {
		if (!(f = gettag(ctx.feedtype, t, tl)))
			f = &notag;
		memcpy(&(ctx.tag), f, sizeof(ctx.tag));
	}

	ctx.iscontenttag = (fieldmap[ctx.tag.id] == FeedFieldContent);
	string_clear(&attrispermalink);
	string_clear(&attrrel);
	string_clear(&attrtype);
}

static void
xmltagstartparsed(XMLParser *p, const char *t, size_t tl, int isshort)
{
	enum TagId tagid;

	if (ISINCONTENT(ctx)) {
		if (ctx.contenttype == ContentTypeHTML) {
			if (isshort)
				xmldata(p, "/>", 2);
			else
				xmldata(p, ">", 1);
		}
		return;
	}

	/* set tag type based on its attribute value */
	if (ctx.tag.id == RSSTagGuid) {
		/* if empty the default is "true" */
		if (!attrispermalink.len ||
		    isattr(attrispermalink.data, attrispermalink.len, STRP("true")))
			ctx.tag.id = RSSTagGuidPermalinkTrue;
		else
			ctx.tag.id = RSSTagGuidPermalinkFalse;
	} else if (ctx.tag.id == AtomTagLink) {
		/* empty or "alternate": other types could be
		   "enclosure", "related", "self" or "via" */
		if (!attrrel.len || isattr(attrrel.data, attrrel.len, STRP("alternate")))
			ctx.tag.id = AtomTagLinkAlternate;
		else if (isattr(attrrel.data, attrrel.len, STRP("enclosure")))
			ctx.tag.id = AtomTagLinkEnclosure;
		else
			ctx.tag.id = AtomTagLink; /* unknown */
	}

	tagid = ctx.tag.id;

	/* map tag type to field: unknown or lesser priority is ignored,
	   when tags of the same type are repeated only the first is used. */
	if (fieldmap[tagid] == -1 ||
	    (!ISFEEDFIELDMULTI(fieldmap[tagid]) &&
	     tagid <= ctx.fields[fieldmap[tagid]].tagid)) {
		return;
	}

	if (ctx.iscontenttag) {
		ctx.iscontent = 1;
		ctx.iscontenttag = 0;

		/* detect content-type based on type attribute */
		if (attrtype.len) {
			if (isattr(attrtype.data, attrtype.len, STRP("html")) ||
			    isattr(attrtype.data, attrtype.len, STRP("xhtml")) ||
			    isattr(attrtype.data, attrtype.len, STRP("text/html")) ||
			    isattr(attrtype.data, attrtype.len, STRP("text/xhtml")) ||
			    isattr(attrtype.data, attrtype.len, STRP("application/xhtml+xml")))
				ctx.contenttype = ContentTypeHTML;
			else /* unknown: handle as base64 text data */
				ctx.contenttype = ContentTypePlain;
		} else {
			/* default content-type */
			if (tagid == RSSTagContentEncoded || tagid == RSSTagDescription)
				ctx.contenttype = ContentTypeHTML;
			else
				ctx.contenttype = ContentTypePlain;
		}
	}

	ctx.field = &(ctx.fields[fieldmap[tagid]].str);
	ctx.fields[fieldmap[tagid]].tagid = tagid;

	/* clear field if it is overwritten (with a priority order) for the new
	   value, if the field can have multiple values then do not clear it. */
	if (!ISFEEDFIELDMULTI(fieldmap[ctx.tag.id]))
		string_clear(ctx.field);
}

static void
xmltagend(XMLParser *p, const char *t, size_t tl, int isshort)
{
	size_t i;

	if (ctx.feedtype == FeedTypeNone)
		return;

	if (ISINCONTENT(ctx)) {
		/* not a closed content field */
		if (!istag(ctx.tag.name, ctx.tag.len, t, tl)) {
			if (!isshort && ctx.contenttype == ContentTypeHTML) {
				xmldata(p, "</", 2);
				xmldata(p, t, tl);
				xmldata(p, ">", 1);
			}
			return;
		}
	} else if (ctx.tag.id && istag(ctx.tag.name, ctx.tag.len, t, tl)) {
		/* matched tag end: close it */
		/* copy also to the link field if the attribute isPermaLink="true"
		   and it is not set by a tag with higher priority. */
		if (ctx.tag.id == RSSTagGuidPermalinkTrue && ctx.field &&
		    ctx.tag.id > ctx.fields[FeedFieldLink].tagid) {
			string_clear(&ctx.fields[FeedFieldLink].str);
			string_append(&ctx.fields[FeedFieldLink].str,
			              ctx.field->data, ctx.field->len);
			ctx.fields[FeedFieldLink].tagid = ctx.tag.id;
		}
	} else if (!ctx.tag.id && ((ctx.feedtype == FeedTypeAtom &&
	   istag(t, tl, STRP("entry"))) || /* Atom */
	   (ctx.feedtype == FeedTypeRSS &&
	   istag(t, tl, STRP("item"))))) /* RSS */
	{
		/* end of RSS or Atom entry / item */
		printfields();

		/* clear strings */
		for (i = 0; i < FeedFieldLast; i++) {
			string_clear(&ctx.fields[i].str);
			ctx.fields[i].tagid = TagUnknown;
		}
		ctx.contenttype = ContentTypeNone;
		/* allow parsing of Atom and RSS concatenated in one XML stream. */
		ctx.feedtype = FeedTypeNone;
	} else {
		return; /* not end of field */
	}

	/* temporary string: for fields that cannot be processed
	   directly and need more context, for example by its tag
	   attributes, like the Atom link rel="alternate|enclosure". */
	if (tmpstr.len && ctx.field) {
		if (ISFEEDFIELDMULTI(fieldmap[ctx.tag.id])) {
			if (ctx.field->len)
				string_append(ctx.field, FieldMultiSeparator, 1);
			string_append(ctx.field, tmpstr.data, tmpstr.len);
		} else {
			string_clear(ctx.field);
			string_append(ctx.field, tmpstr.data, tmpstr.len);
		}
	}

	/* close field */
	string_clear(&tmpstr); /* reuse and clear temporary string */

	if (ctx.tag.id == AtomTagAuthorName)
		memcpy(&(ctx.tag), &atomtagauthor, sizeof(ctx.tag)); /* outer tag */
	else
		memcpy(&(ctx.tag), &notag, sizeof(ctx.tag));

	ctx.iscontent = 0;
	ctx.field = NULL;
}

int
main(int argc, char *argv[])
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (argc > 1) {
		if (uri_parse(argv[1], &baseuri) != -1 && baseuri.proto[0])
			baseurl = argv[1];
		else
			errx(1, "baseurl incorrect or too long");
	}

	memcpy(&(ctx.tag), &notag, sizeof(ctx.tag));

	parser.xmlattr = xmlattr;
	parser.xmlattrentity = xmlattrentity;
	parser.xmlattrend = xmlattrend;
	parser.xmlattrstart = xmlattrstart;
	parser.xmlcdata = xmldata;
	parser.xmldata = xmldata;
	parser.xmldataentity = xmldataentity;
	parser.xmltagend = xmltagend;
	parser.xmltagstart = xmltagstart;
	parser.xmltagstartparsed = xmltagstartparsed;

	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);

	checkfileerror(stdin, "<stdin>", 'r');
	checkfileerror(stdout, "<stdout>", 'w');

	return 0;
}
