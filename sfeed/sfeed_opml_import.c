#include <stdio.h>
#include <strings.h>

#include "util.h"
#include "xml.h"

static XMLParser parser; /* XML parser state */
static char text[256], title[256], xmlurl[2048];

static void
printsafe(const char *s)
{
	for (; *s; s++) {
		if (ISCNTRL((unsigned char)*s))
			continue;
		else if (*s == '\\')
			fputs("\\\\", stdout);
		else if (*s == '\'')
			fputs("'\\''", stdout);
		else
			putchar(*s);
	}
}

static void
xmltagstart(XMLParser *p, const char *t, size_t tl)
{
	text[0] = title[0] = xmlurl[0] = '\0';
}

static void
xmltagend(XMLParser *p, const char *t, size_t tl, int isshort)
{
	if (strcasecmp(t, "outline"))
		return;

	if (xmlurl[0]) {
		fputs("\tfeed '", stdout);
		/* prefer title over text attribute */
		if (title[0])
			printsafe(title);
		else if (text[0])
			printsafe(text);
		else
			fputs("unnamed", stdout);
		fputs("' '", stdout);
		printsafe(xmlurl);
		fputs("'\n", stdout);
	}

	text[0] = title[0] = xmlurl[0] = '\0';
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	if (strcasecmp(t, "outline"))
		return;

	if (!strcasecmp(n, "text"))
		strlcat(text, v, sizeof(text));
	else if (!strcasecmp(n, "title"))
		strlcat(title, v, sizeof(title));
	else if (!strcasecmp(n, "xmlurl"))
		strlcat(xmlurl, v, sizeof(xmlurl));
}

static void
xmlattrentity(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	char buf[8];
	int len;

	if ((len = xml_entitytostr(v, buf, sizeof(buf))) > 0)
		xmlattr(p, t, tl, n, nl, buf, len);
	else
		xmlattr(p, t, tl, n, nl, v, vl);
}

int
main(void)
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	parser.xmlattr = xmlattr;
	parser.xmlattrentity = xmlattrentity;
	parser.xmltagstart = xmltagstart;
	parser.xmltagend = xmltagend;

	fputs(
	    "#sfeedpath=\"$HOME/.sfeed/feeds\"\n"
	    "\n"
	    "# list of feeds to fetch:\n"
	    "feeds() {\n"
	    "	# feed <name> <feedurl> [basesiteurl] [encoding]\n", stdout);
	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);
	fputs("}\n", stdout);

	checkfileerror(stdin, "<stdin>", 'r');
	checkfileerror(stdout, "<stdout>", 'w');

	return 0;
}
