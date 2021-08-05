#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "common.h"

static char bufout[256];
static Item *curentry;
static char cmd;
int lines, columns;

static void
viewsize(int *ln, int *col)
{
	struct winsize ws;

	if (ioctl(1, TIOCGWINSZ, &ws) < 0) {
		die("Could not get terminal resolution: %s",
		    strerror(errno));
	}

	if (ln)
		*ln = ws.ws_row-1; /* one off for status bar */
	if (col)
		*col = ws.ws_col;
}

void
uisetup(void)
{
	viewsize(&lines, &columns);
}

void
uicleanup(void)
{
	return;
}

void
help(void)
{
	puts("Commands:\n"
	     "#: browse item number #.\n"
	     "U: print page URI.\n"
	     "u#: print item number # URI.\n"
	     "0: browse previous item.\n"
	     "n: show next page.\n"
	     "p: show previous page.\n"
	     "t: go to the top of the page\n"
	     "b: go to the bottom of the page\n"
	     "/str: search for string \"str\"\n"
	     "!: refetch failed item.\n"
	     "^D, q: quit.\n"
	     "h, ?: this help.");
}

static int
ndigits(size_t n)
{
	return (n < 10) ? 1 : (n < 100) ? 2 : 3;
}

void
uistatus(char *fmt, ...)
{
	va_list arg;
	int n;

	va_start(arg, fmt);
	n = vsnprintf(bufout, sizeof(bufout), fmt, arg);
	va_end(arg);

	if (n < sizeof(bufout)-1) {
		n += snprintf(bufout + n, sizeof(bufout) - n,
		              " [Press Enter to continue \xe2\x98\x83]");
	}
	if (n >= sizeof(bufout))
		bufout[sizeof(bufout)-1] = '\0';

	mbsprint(bufout, columns);
	fflush(stdout);

	getchar();
}

static void
printstatus(Item *item, char c)
{
	Dir *dir = item->dat;
	char *fmt;
	size_t nitems = dir ? dir->nitems : 0;
	unsigned long long printoff = dir ? dir->printoff : 0;

	fmt = (strcmp(item->port, "70") && strcmp(item->port, "gopher")) ?
	      "%1$3lld%%%*2$3$c %4$s:%8$s/%5$c%6$s [%7$c]: " :
              "%3lld%%%*c %s/%c%s [%c]: ";
	if (snprintf(bufout, sizeof(bufout), fmt,
	             (printoff + lines-1 >= nitems) ? 100 :
	             (printoff + lines) * 100 / nitems, ndigits(nitems)+2, '|',
	             item->host, item->type, item->selector, c, item->port)
	    >= sizeof(bufout))
		bufout[sizeof(bufout)-1] = '\0';
	mbsprint(bufout, columns);
}

char *
uiprompt(char *fmt, ...)
{
	va_list ap;
	char *input = NULL;
	size_t n = 0;
	ssize_t r;

	va_start(ap, fmt);
	if (vsnprintf(bufout, sizeof(bufout), fmt, ap) >= sizeof(bufout))
		bufout[sizeof(bufout)-1] = '\0';
	va_end(ap);

	mbsprint(bufout, columns);
	fflush(stdout);

	if ((r = getline(&input, &n, stdin)) < 0) {
		clearerr(stdin);
		clear(&input);
		putchar('\n');
	} else if (input[r - 1] == '\n') {
		input[--r] = '\0';
	}

	return input;
}

void
uidisplay(Item *entry)
{
	Item *items;
	Dir *dir;
	size_t i, nlines, nitems;
	int nd;

	if (!entry ||
	    !(entry->type == '1' || entry->type == '+' || entry->type == '7') ||
	    !(dir = entry->dat))
		return;

	curentry = entry;

	items = dir->items;
	nitems = dir->nitems;
	nlines = dir->printoff + lines;
	nd = ndigits(nitems);

	for (i = dir->printoff; i < nitems && i < nlines; ++i) {
		if (snprintf(bufout, sizeof(bufout), "%*zu %s %s",
		             nd, i+1, typedisplay(items[i].type),
		             items[i].username)
		    >= sizeof(bufout))
			bufout[sizeof(bufout)-1] = '\0';
		mbsprint(bufout, columns);
		putchar('\n');
	}

	fflush(stdout);
}

void
printuri(Item *item, size_t i)
{
	int n;

	if (!item)
		return;

	switch (item->type) {
	case 0:
		return;
	case '8':
		n = snprintf(bufout, sizeof(bufout), "telnet://%s@%s:%s",
		             item->selector, item->host, item->port);
		break;
	case 'i':
		n = snprintf(bufout, sizeof(bufout), "%zu: %s",
		             i, item->username);
		break;
	case 'h':
		n = snprintf(bufout, sizeof(bufout), "%zu: %s: %s",
		         i, item->username, item->selector);
		break;
	case 'T':
		n = snprintf(bufout, sizeof(bufout), "tn3270://%s@%s:%s",
		             item->selector, item->host, item->port);
		break;
	default:
		n = snprintf(bufout, sizeof(bufout), "%zu: ", i);

		if (n < sizeof(bufout) && *item->username) {
			n += snprintf(bufout+n, sizeof(bufout)-n, "%s: ",
			              item->username);
		}
		if (n < sizeof(bufout)) {
			n += snprintf(bufout+n, sizeof(bufout)-n, "gopher://%s",
			              item->host);
		}
		if (n < sizeof(bufout) && strcmp(item->port, "70")) {
			n += snprintf(bufout+n, sizeof(bufout)-n, ":%s",
			              item->port);
		}
		if (n < sizeof(bufout)) {
			n += snprintf(bufout+n, sizeof(bufout)-n, "/%c%s",
			              item->type, item->selector);
		}
		if (n < sizeof(bufout) && item->type == '7' && item->tag) {
			n += snprintf(bufout+n, sizeof(bufout)-n, "%%09%s",
			              item->tag + strlen(item->selector));
		}
		break;
	}

	if (n >= sizeof(bufout))
		bufout[sizeof(bufout)-1] = '\0';

	mbsprint(bufout, columns);
	putchar('\n');
}

void
searchinline(const char *searchstr, Item *entry)
{
	Dir *dir;
	size_t i;

	if (!searchstr || !*searchstr || !(dir = entry->dat))
		return;

	for (i = 0; i < dir->nitems; ++i)
		if (strcasestr(dir->items[i].username, searchstr))
			printuri(&(dir->items[i]), i + 1);
}

Item *
uiselectitem(Item *entry)
{
	Dir *dir;
	char buf[BUFSIZ], *sstr, nl;
	int item, nitems;

	if (!entry || !(dir = entry->dat))
		return NULL;

	nitems = dir ? dir->nitems : 0;

	for (;;) {
		if (!cmd)
			cmd = 'h';
		printstatus(entry, cmd);
		fflush(stdout);

		if (!fgets(buf, sizeof(buf), stdin)) {
			putchar('\n');
			return NULL;
		}
		if (isdigit((unsigned char)*buf)) {
			cmd = '\0';
			nl = '\0';
			if (sscanf(buf, "%d%c", &item, &nl) != 2 || nl != '\n')
				item = -1;
		} else if (!strcmp(buf+1, "\n")) {
			item = -1;
			cmd = *buf;
		} else if (*buf == '/') {
			for (sstr = buf+1; *sstr && *sstr != '\n'; ++sstr)
			     ;
			*sstr = '\0';
			sstr = buf+1;
			cmd = *buf;
		} else if (isdigit((unsigned char)*(buf+1))) {
			nl = '\0';
			if (sscanf(buf+1, "%d%c", &item, &nl) != 2 || nl != '\n')
				item = -1;
			else
				cmd = *buf;
		}

		switch (cmd) {
		case '\0':
			break;
		case 'q':
			return NULL;
		case 'n':
			if (lines < nitems - dir->printoff &&
			    lines < (size_t)-1 - dir->printoff)
				dir->printoff += lines;
			return entry;
		case 'p':
			if (lines <= dir->printoff)
				dir->printoff -= lines;
			else
				dir->printoff = 0;
			return entry;
		case 'b':
			if (nitems > lines)
				dir->printoff = nitems - lines;
			else
				dir->printoff = 0;
			return entry;
		case 't':
			dir->printoff = 0;
			return entry;
		case '!':
			if (entry->raw)
				continue;
			return entry;
		case 'U':
			printuri(entry, 0);
			continue;
		case 'u':
			if (item > 0 && item <= nitems)
				printuri(&dir->items[item-1], item);
			continue;
		case '/':
			if (*sstr)
				searchinline(sstr, entry);
			continue;
		case 'h':
		case '?':
			help();
			continue;
		default:
			cmd = 'h';
			continue;
		}

		if (item >= 0 && item <= nitems)
			break;
	}

	if (item > 0)
		return &dir->items[item-1];

	return entry->entry;
}

void
uisigwinch(int signal)
{
	uisetup();

	if (!curentry)
		return;

	putchar('\n');
	uidisplay(curentry);
	printstatus(curentry, cmd);
	fflush(stdout);
}
