#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"

static char *line;
static size_t linesize;
static int firstitem = 1;

/* Unescape / decode fields printed by string_print_encoded() */
static void
printcontent(const char *s)
{
	for (; *s; s++) {
		switch (*s) {
		case '\\':
			if (*(s + 1) == '\0')
				break;
			s++;
			switch (*s) {
			case 'n':  fputs("\\n",  stdout); break;
			case '\\': fputs("\\\\", stdout); break;
			case 't':  fputs("\\t",  stdout); break;
			}
			break; /* ignore invalid escape sequence */
		case '"': fputs("\\\"", stdout); break;
		default:
			putchar(*s);
			break;
		}
	}
}

static void
printfield(const char *s)
{
	for (; *s; s++) {
		if (*s == '\\')
			fputs("\\\\", stdout);
		else if (*s == '"')
			fputs("\\\"", stdout);
		else
			putchar(*s);
	}
}

static void
printfeed(FILE *fp, const char *feedname)
{
	char *fields[FieldLast], timebuf[32];
	struct tm parsedtm, *tm;
	time_t parsedtime;
	ssize_t linelen;
	int ch;
	char *p, *s;

	while ((linelen = getline(&line, &linesize, fp)) > 0 &&
	       !ferror(stdout)) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		parseline(line, fields);

		if (!firstitem)
			fputs(",\n", stdout);
		firstitem = 0;

		fputs("{\n\t\"id\": \"", stdout);
		printfield(fields[FieldId]);
		fputs("\"", stdout);

		parsedtime = 0;
		if (!strtotime(fields[FieldUnixTimestamp], &parsedtime) &&
		    (tm = gmtime_r(&parsedtime, &parsedtm)) &&
		    strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", tm)) {
			fputs(",\n\t\"date_published\": \"", stdout);
			fputs(timebuf, stdout);
			fputs("\"", stdout);
		}

		fputs(",\n\t\"title\": \"", stdout);
		if (feedname[0]) {
			fputs("[", stdout);
			printfield(feedname);
			fputs("] ", stdout);
		}
		printfield(fields[FieldTitle]);
		fputs("\"", stdout);

		if (fields[FieldLink][0]) {
			fputs(",\n\t\"url\": \"", stdout);
			printfield(fields[FieldLink]);
			fputs("\"", stdout);
		}

		if (fields[FieldAuthor][0]) {
			fputs(",\n\t\"authors\": [{\"name\": \"", stdout);
			printfield(fields[FieldAuthor]);
			fputs("\"}]", stdout);
		}

		if (fields[FieldCategory][0]) {
			fputs(",\n\t\"tags\": [", stdout);

			for (p = s = fields[FieldCategory]; ; s++) {
				if (*s == '|' || *s == '\0') {
					if (p != fields[FieldCategory])
						fputs(", ", stdout);
					ch = *s;
					*s = '\0'; /* temporary NUL terminate */
					fputs("\"", stdout);
					printfield(p);
					fputs("\"", stdout);
					*s = ch; /* restore */
					p = s + 1;
				}
				if (*s == '\0')
					break;
			}
			fputs("]", stdout);
		}

		if (fields[FieldEnclosure][0]) {
			fputs(",\n\t\"attachments\": [{\"url:\": \"", stdout);
			printfield(fields[FieldEnclosure]);
			fputs("\"}]", stdout);
		}

		if (!strcmp(fields[FieldContentType], "html"))
			fputs(",\n\t\"content_html\": \"", stdout);
		else
			fputs(",\n\t\"content_text\": \"", stdout);
		printcontent(fields[FieldContent]);
		fputs("\"\n}", stdout);
	}
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *name;
	int i;

	if (pledge(argc == 1 ? "stdio" : "stdio rpath", NULL) == -1)
		err(1, "pledge");

	fputs("{\n"
	      "\"version\": \"https://jsonfeed.org/version/1.1\",\n"
	      "\"title\": \"Newsfeed\",\n"
	      "\"items\": [\n", stdout);

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
	fputs("]\n}\n", stdout);

	checkfileerror(stdout, "<stdout>", 'w');

	return 0;
}
