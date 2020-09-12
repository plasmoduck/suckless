#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

static char *line;
static size_t linesize;
static char host[256], *user, dtimebuf[32], mtimebuf[32];

static unsigned long
djb2(unsigned char *s, unsigned long hash)
{
	int c;

	while ((c = *s++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

static void
printfeed(FILE *fp, const char *feedname)
{
	char *fields[FieldLast], timebuf[32];
	struct tm *tm;
	time_t parsedtime;
	unsigned long hash;
	ssize_t linelen;

	while ((linelen = getline(&line, &linesize, fp)) > 0) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		hash = djb2((unsigned char *)line, 5381UL);
		parseline(line, fields);

		/* mbox + mail header */
		printf("From MAILER-DAEMON %s\n", mtimebuf);

		parsedtime = 0;
		if (!strtotime(fields[FieldUnixTimestamp], &parsedtime) &&
		    (tm = gmtime(&parsedtime)) &&
		    strftime(timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S +0000", tm)) {
			printf("Date: %s\n", timebuf);
		} else {
			printf("Date: %s\n", dtimebuf); /* invalid/missing: use current time */
		}

		printf("From: %s <sfeed@>\n", fields[FieldAuthor][0] ? fields[FieldAuthor] : feedname);
		printf("To: %s <%s@%s>\n", user, user, host);
		printf("Subject: %s\n", fields[FieldTitle]);
		printf("Message-ID: <%s%s%lu@%s>\n",
		       fields[FieldUnixTimestamp],
		       fields[FieldUnixTimestamp][0] ? "." : "",
		       hash, feedname);
		printf("Content-Type: text/plain; charset=\"utf-8\"\n");
		printf("Content-Transfer-Encoding: binary\n");
		printf("X-Feedname: %s\n\n", feedname);

		printf("%s\n", fields[FieldLink]);
		if (fields[FieldEnclosure][0])
			printf("\nEnclosure:\n%s\n", fields[FieldEnclosure]);
		fputs("\n", stdout);
	}
}

int
main(int argc, char *argv[])
{
	struct tm tmnow;
	time_t now;
	FILE *fp;
	char *name;
	int i;

	if (pledge(argc == 1 ? "stdio" : "stdio rpath", NULL) == -1)
		err(1, "pledge");

	if (!(user = getenv("USER")))
		user = "you";
	if (gethostname(host, sizeof(host)) == -1)
		err(1, "gethostname");
	if ((now = time(NULL)) == -1)
		err(1, "time");
	if (!gmtime_r(&now, &tmnow))
		err(1, "gmtime_r: can't get current time");
	if (!strftime(mtimebuf, sizeof(mtimebuf), "%a %b %d %H:%M:%S %Y", &tmnow))
		errx(1, "strftime: can't format current time");
	if (!strftime(dtimebuf, sizeof(dtimebuf), "%a, %d %b %Y %H:%M:%S +0000", &tmnow))
		errx(1, "strftime: can't format current time");

	if (argc == 1) {
		printfeed(stdin, "");
	} else {
		for (i = 1; i < argc; i++) {
			if (!(fp = fopen(argv[i], "r")))
				err(1, "fopen: %s", argv[i]);
			name = ((name = strrchr(argv[i], '/'))) ? name + 1 : argv[i];
			printfeed(fp, name);
			if (ferror(fp))
				err(1, "ferror: %s", argv[i]);
			fclose(fp);
		}
	}
	return 0;
}
