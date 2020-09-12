#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <strings.h>

#include "util.h"
#include "xml.h"

/* string and size */
#define STRP(s) s,sizeof(s)-1

static XMLParser parser;
static int isbase, islink, isfeedlink;
static char abslink[4096], feedlink[4096], basehref[4096], feedtype[256];

static void
printfeedtype(const char *s, FILE *fp)
{
	for (; *s; s++)
		if (!isspace((unsigned char)*s))
			fputc(*s, fp);
}

static void
xmltagstart(XMLParser *p, const char *t, size_t tl)
{
	isbase = islink = isfeedlink = 0;

	if (!strcasecmp(t, "base"))
		isbase = 1;
	else if (!strcasecmp(t, "link"))
		islink = 1;
}

static void
xmltagstartparsed(XMLParser *p, const char *t, size_t tl, int isshort)
{
	if (!isfeedlink)
		return;

	if (absuri(abslink, sizeof(abslink), feedlink, basehref) != -1)
		fputs(abslink, stdout);
	else
		fputs(feedlink, stdout);
	putchar('\t');
	printfeedtype(feedtype, stdout);
	putchar('\n');
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	if (isbase) {
		if (!strcasecmp(n, "href"))
			strlcpy(basehref, v, sizeof(basehref));
	} else if (islink) {
		if (!strcasecmp(n, "type")) {
			if (!strncasecmp(v, STRP("application/atom")) ||
			    !strncasecmp(v, STRP("application/xml"))  ||
			    !strncasecmp(v, STRP("application/rss"))) {
				isfeedlink = 1;
				strlcpy(feedtype, v, sizeof(feedtype));
			}
		} else if (!strcasecmp(n, "href")) {
			strlcpy(feedlink, v, sizeof(feedlink));
		}
	}
}

int
main(int argc, char *argv[])
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (argc > 1)
		strlcpy(basehref, argv[1], sizeof(basehref));

	parser.xmlattr = xmlattr;
	parser.xmltagstart = xmltagstart;
	parser.xmltagstartparsed = xmltagstartparsed;

	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);

	return 0;
}
