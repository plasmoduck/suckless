#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"

static char *line;
static size_t linesize;

static void
printfeed(FILE *fp, const char *feedname)
{
	char *fields[FieldLast];
	struct tm parsedtm, *tm;
	time_t parsedtime;
	ssize_t linelen;

	while ((linelen = getline(&line, &linesize, fp)) > 0 &&
	       !ferror(stdout)) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		parseline(line, fields);

		parsedtime = 0;
		if (!strtotime(fields[FieldUnixTimestamp], &parsedtime) &&
		    (tm = gmtime_r(&parsedtime, &parsedtm))) {
			fprintf(stdout, "%04d-%02d-%02dT%02d:%02d:%02dZ\t",
			        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			        tm->tm_hour, tm->tm_min, tm->tm_sec);
		} else {
			fputs("\t", stdout);
		}
		if (feedname[0])
			printf("[%s] ", feedname);
		fputs(fields[FieldTitle], stdout);
		if (fields[FieldLink][0]) {
			fputs(": ", stdout);
			fputs(fields[FieldLink], stdout);
		}
		putchar('\n');
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

	checkfileerror(stdout, "<stdout>", 'w');

	return 0;
}
