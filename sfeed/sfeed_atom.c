#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"

static struct tm tmnow;
static time_t now;
static char *line;
static size_t linesize;

static void
printcontent(const char *s)
{
	for (; *s; ++s) {
		switch (*s) {
		case '<':  fputs("&lt;",   stdout); break;
		case '>':  fputs("&gt;",   stdout); break;
		case '\'': fputs("&#39;",  stdout); break;
		case '&':  fputs("&amp;",  stdout); break;
		case '"':  fputs("&quot;", stdout); break;
		case '\\':
			if (*(s + 1) == '\0')
				break;
			s++;
			switch (*s) {
			case 'n':  putchar('\n'); break;
			case '\\': putchar('\\'); break;
			case 't':  putchar('\t'); break;
			}
			break;
		default:  putchar(*s);
		}
	}
}

static void
printfeed(FILE *fp, const char *feedname)
{
	char *fields[FieldLast], *p, *tmp;
	struct tm parsedtm, *tm;
	time_t parsedtime;
	ssize_t linelen;
	int c;

	while ((linelen = getline(&line, &linesize, fp)) > 0 &&
	       !ferror(stdout)) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		parseline(line, fields);

		fputs("<entry>\n\t<title>", stdout);
		if (feedname[0]) {
			fputs("[", stdout);
			xmlencode(feedname, stdout);
			fputs("] ", stdout);
		}
		xmlencode(fields[FieldTitle], stdout);
		fputs("</title>\n", stdout);
		if (fields[FieldLink][0]) {
			fputs("\t<link rel=\"alternate\" href=\"", stdout);
			xmlencode(fields[FieldLink], stdout);
			fputs("\" />\n", stdout);
		}
		/* prefer link over id for Atom <id>. */
		fputs("\t<id>", stdout);
		if (fields[FieldLink][0])
			xmlencode(fields[FieldLink], stdout);
		else if (fields[FieldId][0])
			xmlencode(fields[FieldId], stdout);
		fputs("</id>\n", stdout);
		if (fields[FieldEnclosure][0]) {
			fputs("\t<link rel=\"enclosure\" href=\"", stdout);
			xmlencode(fields[FieldEnclosure], stdout);
			fputs("\" />\n", stdout);
		}

		parsedtime = 0;
		if (strtotime(fields[FieldUnixTimestamp], &parsedtime) ||
		    !(tm = gmtime_r(&parsedtime, &parsedtm)))
			tm = &tmnow;
		fprintf(stdout, "\t<updated>%04d-%02d-%02dT%02d:%02d:%02dZ</updated>\n",
		        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		        tm->tm_hour, tm->tm_min, tm->tm_sec);

		if (fields[FieldAuthor][0]) {
			fputs("\t<author><name>", stdout);
			xmlencode(fields[FieldAuthor], stdout);
			fputs("</name></author>\n", stdout);
		}
		if (fields[FieldContent][0]) {
			if (!strcmp(fields[FieldContentType], "html")) {
				fputs("\t<content type=\"html\">", stdout);
			} else {
				/* NOTE: an RSS/Atom viewer may or may not format
				   whitespace such as newlines.
				   Workaround: type="html" and <![CDATA[<pre></pre>]]> */
				fputs("\t<content>", stdout);
			}
			printcontent(fields[FieldContent]);
			fputs("</content>\n", stdout);
		}
		for (p = fields[FieldCategory]; (tmp = strchr(p, '|')); p = tmp + 1) {
			c = *tmp;
			*tmp = '\0'; /* temporary NUL-terminate */
			if (*p) {
				fputs("\t<category term=\"", stdout);
				xmlencode(p, stdout);
				fputs("\" />\n", stdout);
			}
			*tmp = c; /* restore */
		}
		fputs("</entry>\n", stdout);
	}
}

int
main(int argc, char *argv[])
{
	struct tm *tm;
	FILE *fp;
	char *name;
	int i;

	if (pledge(argc == 1 ? "stdio" : "stdio rpath", NULL) == -1)
		err(1, "pledge");

	if ((now = time(NULL)) == (time_t)-1)
		errx(1, "time");
	if (!(tm = gmtime_r(&now, &tmnow)))
		err(1, "gmtime_r: can't get current time");

	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	      "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
	      "\t<title>Newsfeed</title>\n", stdout);
	printf("\t<id>urn:newsfeed:%lld</id>\n"
	       "\t<updated>%04d-%02d-%02dT%02d:%02d:%02dZ</updated>\n",
	       (long long)now,
	       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	       tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (argc == 1) {
		printfeed(stdin, "");
		checkfileerror(stdin, "<stdin>", 'r');
	} else {
		for (i = 1; i < argc; i++) {
			if (!(fp = fopen(argv[i], "r")))
				err(1, "fopen: %s", argv[i]);
			name = ((name = strrchr(argv[i], '/'))) ? name + 1 : argv[i];
			printfeed(fp, name);
			checkfileerror(fp, argv[i], 'r');
			checkfileerror(stdout, "<stdout>", 'w');
			fclose(fp);
		}
	}

	fputs("</feed>\n", stdout);

	checkfileerror(stdout, "<stdout>", 'w');

	return 0;
}
