#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "util.h"

static char *prefixpath = "/", *host = "127.0.0.1", *port = "70"; /* default */
static char *line;
static size_t linesize;
static time_t comparetime;

/* Escape characters in gopher, CR and LF are ignored */
void
gophertext(FILE *fp, const char *s)
{
	for (; *s; s++) {
		switch (*s) {
		case '\r': /* ignore CR */
		case '\n': /* ignore LF */
			break;
		case '\t':
			fputs("        ", fp);
			break;
		default:
			putc(*s, fp);
			break;
		}
	}
}

static void
printfeed(FILE *fpitems, FILE *fpin, struct feed *f)
{
	struct uri u;
	char *fields[FieldLast];
	char *itemhost, *itemport, *itempath, *itemquery, *itemfragment;
	ssize_t linelen;
	unsigned int isnew;
	struct tm rtm, *tm;
	time_t parsedtime;
	int itemtype;

	if (f->name[0]) {
		fprintf(fpitems, "i%s\t\t%s\t%s\r\n", f->name, host, port);
		fprintf(fpitems, "i\t\t%s\t%s\r\n", host, port);
	}

	while ((linelen = getline(&line, &linesize, fpin)) > 0 &&
	       !ferror(fpitems)) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		parseline(line, fields);

		itemhost = host;
		itemport = port;
		itemtype = 'i';
		itempath = fields[FieldLink];
		itemquery = "";
		itemfragment = "";

		if (fields[FieldLink][0]) {
			itemtype = 'h';
			/* if it is a gopher URL then change it into a DirEntity */
			if (!strncmp(fields[FieldLink], "gopher://", 9) &&
			    uri_parse(fields[FieldLink], &u) != -1) {
				itemhost = u.host;
				itemport = u.port[0] ? u.port : "70";
				itemtype = '1';
				itempath = u.path;
				itemquery = u.query;
				itemfragment = u.fragment;

				if (itempath[0] == '/') {
					itempath++;
					if (*itempath) {
						itemtype = *itempath;
						itempath++;
					}
				}
			}
		}

		parsedtime = 0;
		if (!strtotime(fields[FieldUnixTimestamp], &parsedtime) &&
		    (tm = localtime_r(&parsedtime, &rtm))) {
			isnew = (parsedtime >= comparetime) ? 1 : 0;
			f->totalnew += isnew;

			fprintf(fpitems, "%c%c %04d-%02d-%02d %02d:%02d ",
			        itemtype,
			        isnew ? 'N' : ' ',
			        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			        tm->tm_hour, tm->tm_min);
		} else {
			fprintf(fpitems, "%c                   ", itemtype);
		}
		f->total++;

		gophertext(fpitems, fields[FieldTitle]);
		fputs("\t", fpitems);
		if (itemtype == 'h' && fields[FieldLink] == itempath)
			fputs("URL:", fpitems);
		gophertext(fpitems, itempath);
		if (itemquery[0]) {
			fputs("?", fpitems);
			gophertext(fpitems, itemquery);
		}
		if (itemfragment[0]) {
			fputs("#", fpitems);
			gophertext(fpitems, itemfragment);
		}
		fprintf(fpitems, "\t%s\t%s\r\n", itemhost, itemport);
	}
	fputs(".\r\n", fpitems);
}

int
main(int argc, char *argv[])
{
	struct feed f;
	FILE *fpitems, *fpindex, *fp;
	char *name, *p;
	int i;

	if (argc == 1) {
		if (pledge("stdio", NULL) == -1)
			err(1, "pledge");
	} else {
		if (unveil("/", "r") == -1)
			err(1, "unveil: /");
		if (unveil(".", "rwc") == -1)
			err(1, "unveil: .");
		if (pledge("stdio rpath wpath cpath", NULL) == -1)
			err(1, "pledge");
	}

	if ((comparetime = getcomparetime()) == (time_t)-1)
		errx(1, "getcomparetime");

	if ((p = getenv("SFEED_GOPHER_HOST")))
		host = p;
	if ((p = getenv("SFEED_GOPHER_PORT")))
		port = p;

	if (argc == 1) {
		f.name = "";
		printfeed(stdout, stdin, &f);
		checkfileerror(stdin, "<stdin>", 'r');
		checkfileerror(stdout, "<stdout>", 'w');
	} else {
		if ((p = getenv("SFEED_GOPHER_PATH")))
			prefixpath = p;

		/* write main index page */
		if (!(fpindex = fopen("index", "wb")))
			err(1, "fopen: index");

		for (i = 1; i < argc; i++) {
			memset(&f, 0, sizeof(f));
			name = ((name = strrchr(argv[i], '/'))) ? name + 1 : argv[i];
			f.name = name;

			if (!(fp = fopen(argv[i], "r")))
				err(1, "fopen: %s", argv[i]);
			if (!(fpitems = fopen(name, "wb")))
				err(1, "fopen");
			printfeed(fpitems, fp, &f);
			checkfileerror(fp, argv[i], 'r');
			checkfileerror(fpitems, name, 'w');
			fclose(fp);
			fclose(fpitems);

			/* append directory item to index */
			fputs("1", fpindex);
			gophertext(fpindex, name);
			fprintf(fpindex, " (%lu/%lu)\t", f.totalnew, f.total);
			gophertext(fpindex, prefixpath);
			gophertext(fpindex, name);
			fprintf(fpindex, "\t%s\t%s\r\n", host, port);
		}
		fputs(".\r\n", fpindex);
		checkfileerror(fpindex, "index", 'w');
		fclose(fpindex);
	}

	return 0;
}
