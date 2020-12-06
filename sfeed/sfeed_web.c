#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <strings.h>

#include "util.h"
#include "xml.h"

/* string and size */
#define STRP(s) s,sizeof(s)-1

static XMLParser parser;
static int isbasetag, islinktag, ishrefattr, istypeattr;
static char linkhref[4096], linktype[256], basehref[4096];
static char abslink[4096];

static void
printvalue(const char *s)
{
	for (; *s; s++)
		if (!iscntrl((unsigned char)*s))
			putchar(*s);
}

static void
xmltagstart(XMLParser *p, const char *t, size_t tl)
{
	isbasetag = islinktag = 0;

	if (!strcasecmp(t, "base")) {
		isbasetag = 1;
	} else if (!strcasecmp(t, "link")) {
		islinktag = 1;
		linkhref[0] = '\0';
		linktype[0] = '\0';
	}
}

static void
xmltagstartparsed(XMLParser *p, const char *t, size_t tl, int isshort)
{
	if (!islinktag)
		return;

	if (strncasecmp(linktype, STRP("application/atom")) &&
	    strncasecmp(linktype, STRP("application/xml")) &&
	    strncasecmp(linktype, STRP("application/rss")))
		return;

	if (absuri(abslink, sizeof(abslink), linkhref, basehref) != -1)
		printvalue(abslink);
	else
		printvalue(linkhref);
	putchar('\t');
	printvalue(linktype);
	putchar('\n');
}

static void
xmlattrstart(XMLParser *p, const char *t, size_t tl, const char *a, size_t al)
{
	ishrefattr = istypeattr = 0;

	if (!isbasetag && !islinktag)
		return;

	if (!strcasecmp(a, "href")) {
		ishrefattr = 1;
		if (isbasetag)
			basehref[0] = '\0';
		else if (islinktag)
			linkhref[0] = '\0';
	} else if (!strcasecmp(a, "type") && islinktag) {
		istypeattr = 1;
		linktype[0] = '\0';
	}
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	if (isbasetag && ishrefattr) {
		strlcat(basehref, v, sizeof(basehref));
	} else if (islinktag) {
		if (ishrefattr)
			strlcat(linkhref, v, sizeof(linkhref));
		else if (istypeattr)
			strlcat(linktype, v, sizeof(linktype));
	}
}

static void
xmlattrentity(XMLParser *p, const char *t, size_t tl, const char *a, size_t al,
              const char *v, size_t vl)
{
	char buf[16];
	int len;

	if (!ishrefattr && !istypeattr)
		return;

	/* try to translate entity, else just pass as data to
	 * xmlattr handler. */
	if ((len = xml_entitytostr(v, buf, sizeof(buf))) > 0)
		xmlattr(p, t, tl, a, al, buf, (size_t)len);
	else
		xmlattr(p, t, tl, a, al, v, vl);
}

int
main(int argc, char *argv[])
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (argc > 1)
		strlcpy(basehref, argv[1], sizeof(basehref));

	parser.xmlattr = xmlattr;
	parser.xmlattrentity = xmlattrentity;
	parser.xmlattrstart = xmlattrstart;
	parser.xmltagstart = xmltagstart;
	parser.xmltagstartparsed = xmltagstartparsed;

	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);

	return 0;
}
