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
	   start of the data. */
	if (tags++ > 3)
		exit(0);
}

static void
xmlattr(XMLParser *p, const char *t, size_t tl, const char *n, size_t nl,
	const char *v, size_t vl)
{
	if (strcasecmp(t, "?xml") || strcasecmp(n, "encoding"))
		return;

	/* output lowercase, no control characters */
	for (; *v; v++) {
		if (!iscntrl((unsigned char)*v))
			putchar(tolower((unsigned char)*v));
	}
	putchar('\n');
	exit(0);
}

int
main(void)
{
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	parser.xmlattr = xmlattr;
	parser.xmltagstart = xmltagstart;

	/* NOTE: getnext is defined in xml.h for inline optimization */
	xml_parse(&parser);

	return 0;
}
