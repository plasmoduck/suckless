#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

static char *line;
static size_t linesize;
static char host[256], *user, dtimebuf[32], mtimebuf[32];
static int usecontent = 0; /* env variable: $SFEED_MBOX_CONTENT */

static unsigned long long
djb2(unsigned char *s, unsigned long long hash)
{
	int c;

	while ((c = *s++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

/* Unescape / decode fields printed by string_print_encoded()
 * "\\" to "\", "\t", to TAB, "\n" to newline. Other escape sequences are
 * ignored: "\z" etc. Mangle "From " in mboxrd style (always prefix >). */
static void
printcontent(const char *s, FILE *fp)
{
escapefrom:
	for (; *s == '>'; s++)
		putc('>', fp);
	/* escape "From ", mboxrd-style. */
	if (!strncmp(s, "From ", 5))
		putc('>', fp);

	for (; *s; s++) {
		switch (*s) {
		case '\\':
			if (*(s + 1) == '\0')
				break;
			s++;
			switch (*s) {
			case 'n':
				putc('\n', fp);
				s++;
				goto escapefrom;
			case '\\': putc('\\', fp); break;
			case 't':  putc('\t', fp); break;
			}
			break;
		default:
			putc(*s, fp); break;
		}
	}
}

static void
printfeed(FILE *fp, const char *feedname)
{
	char *fields[FieldLast], timebuf[32];
	struct tm parsedtm, *tm;
	time_t parsedtime;
	unsigned long long hash;
	ssize_t linelen;
	int ishtml;

	while ((linelen = getline(&line, &linesize, fp)) > 0 &&
	       !ferror(stdout)) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		hash = djb2((unsigned char *)line, 5381ULL);
		parseline(line, fields);

		/* mbox + mail header */
		printf("From MAILER-DAEMON %s\n", mtimebuf);

		parsedtime = 0;
		if (!strtotime(fields[FieldUnixTimestamp], &parsedtime) &&
		    (tm = gmtime_r(&parsedtime, &parsedtm)) &&
		    strftime(timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S +0000", tm)) {
			printf("Date: %s\n", timebuf);
		} else {
			printf("Date: %s\n", dtimebuf); /* invalid/missing: use current time */
		}

		printf("From: %s <anonymous@>\n", fields[FieldAuthor][0] ? fields[FieldAuthor] : feedname);
		printf("To: %s <%s@%s>\n", user, user, host);
		printf("Subject: %s\n", fields[FieldTitle]);
		printf("Message-ID: <%s%s%llu@%s>\n",
		       fields[FieldUnixTimestamp],
		       fields[FieldUnixTimestamp][0] ? "." : "",
		       hash, feedname);

		ishtml = usecontent && !strcmp(fields[FieldContentType], "html");
		if (ishtml)
			fputs("Content-Type: text/html; charset=\"utf-8\"\n", stdout);
		else
			fputs("Content-Type: text/plain; charset=\"utf-8\"\n", stdout);
		fputs("Content-Transfer-Encoding: binary\n", stdout);
		printf("X-Feedname: %s\n", feedname);
		fputs("\n", stdout);

		if (ishtml) {
			fputs("<p>\n", stdout);
			if (fields[FieldLink][0]) {
				fputs("Link: <a href=\"", stdout);
				xmlencode(fields[FieldLink], stdout);
				fputs("\">", stdout);
				xmlencode(fields[FieldLink], stdout);
				fputs("</a><br/>\n", stdout);
			}
			if (fields[FieldEnclosure][0]) {
				fputs("Enclosure: <a href=\"", stdout);
				xmlencode(fields[FieldEnclosure], stdout);
				fputs("\">", stdout);
				xmlencode(fields[FieldEnclosure], stdout);
				fputs("</a><br/>\n", stdout);
			}
			fputs("</p>\n", stdout);
		} else {
			if (fields[FieldLink][0])
				printf("Link:      %s\n", fields[FieldLink]);
			if (fields[FieldEnclosure][0])
				printf("Enclosure: %s\n", fields[FieldEnclosure]);
		}
		if (usecontent) {
			fputs("\n", stdout);
			if (ishtml && fields[FieldLink][0]) {
				fputs("<base href=\"", stdout);
				xmlencode(fields[FieldLink], stdout);
				fputs("\"/>\n", stdout);
			}
			printcontent(fields[FieldContent], stdout);
		}
		fputs("\n\n", stdout);
	}
}

int
main(int argc, char *argv[])
{
	struct tm tmnow;
	time_t now;
	FILE *fp;
	char *name, *tmp;
	int i;

	if (pledge(argc == 1 ? "stdio" : "stdio rpath", NULL) == -1)
		err(1, "pledge");

	if ((tmp = getenv("SFEED_MBOX_CONTENT")))
		usecontent = !strcmp(tmp, "1");
	if (!(user = getenv("USER")))
		user = "you";
	if (gethostname(host, sizeof(host)) == -1)
		err(1, "gethostname");
	if ((now = time(NULL)) == (time_t)-1)
		errx(1, "time");
	if (!gmtime_r(&now, &tmnow))
		err(1, "gmtime_r: can't get current time");
	if (!strftime(mtimebuf, sizeof(mtimebuf), "%a %b %d %H:%M:%S %Y", &tmnow))
		errx(1, "strftime: can't format current time");
	if (!strftime(dtimebuf, sizeof(dtimebuf), "%a, %d %b %Y %H:%M:%S +0000", &tmnow))
		errx(1, "strftime: can't format current time");

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
