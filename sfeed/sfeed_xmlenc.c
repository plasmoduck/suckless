#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "util.h"
#include "xml.h"

static XMLParser parser;
static int tags;

static void
xmltagstart(XMLParser *p, const char *t, size_t tl)
{
	/* optimization: try to find a processing instruction only at the
	   start of the data at the first few starting tags. */
	if (tags++ > 3)
		exit(0);
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	if (strcasecmp(t, "?xml") || strcasecmp(n, "encoding"))
		return;

	for (; *v; v++) {
		if (isalpha((unsigned char)*v) ||
		    isdigit((unsigned char)*v) ||
		    *v == '.' || *v == ':' || *v == '-' || *v == '_')
			putchar(tolower((unsigned char)*v));
	}
}

static void
xmlattrend(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl)
{
	if (strcasecmp(t, "?xml") || strcasecmp(n, "encoding"))
		return;
	putchar('\n');
	exit(0);
}

int
main(void)
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	parser.xmlattr = xmlattr;
	parser.xmlattrentity = xmlattr; /* no entity conversion */
	parser.xmlattrend = xmlattrend;
	parser.xmltagstart = xmltagstart;

	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);

	return 0;
}
