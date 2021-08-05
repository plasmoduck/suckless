#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

/* curses */
#ifndef SFEED_MINICURSES
#include <curses.h>
#include <term.h>
#else
#include "minicurses.h"
#endif

#define LEN(a)   sizeof((a))/sizeof((a)[0])
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define PAD_TRUNCATE_SYMBOL    "\xe2\x80\xa6" /* symbol: "ellipsis" */
#define SCROLLBAR_SYMBOL_BAR   "\xe2\x94\x82" /* symbol: "light vertical" */
#define SCROLLBAR_SYMBOL_TICK  " "
#define LINEBAR_SYMBOL_BAR     "\xe2\x94\x80" /* symbol: "light horizontal" */
#define LINEBAR_SYMBOL_RIGHT   "\xe2\x94\xa4" /* symbol: "light vertical and left" */
#define UTF_INVALID_SYMBOL     "\xef\xbf\xbd" /* symbol: "replacement" */

/* color-theme */
#ifndef SFEED_THEME
#define SFEED_THEME "themes/mono.h"
#endif
#include SFEED_THEME

enum {
	ATTR_RESET = 0,	ATTR_BOLD_ON = 1, ATTR_FAINT_ON = 2, ATTR_REVERSE_ON = 7
};

enum Layout {
	LayoutVertical = 0, LayoutHorizontal, LayoutMonocle, LayoutLast
};

enum Pane { PaneFeeds, PaneItems, PaneLast };

enum {
	FieldUnixTimestamp = 0, FieldTitle, FieldLink, FieldContent,
	FieldContentType, FieldId, FieldAuthor, FieldEnclosure,
	FieldCategory, FieldLast
};

struct win {
	int width; /* absolute width of the window */
	int height; /* absolute height of the window */
	int dirty; /* needs draw update: clears screen */
};

struct row {
	char *text; /* text string, optional if using row_format() callback */
	int bold;
	void *data; /* data binding */
};

struct pane {
	int x; /* absolute x position on the screen */
	int y; /* absolute y position on the screen */
	int width; /* absolute width of the pane */
	int height; /* absolute height of the pane, should be > 0 */
	off_t pos; /* focused row position */
	struct row *rows;
	size_t nrows; /* total amount of rows */
	int focused; /* has focus or not */
	int hidden; /* is visible or not */
	int dirty; /* needs draw update */
	/* (optional) callback functions */
	struct row *(*row_get)(struct pane *, off_t pos);
	char *(*row_format)(struct pane *, struct row *);
	int (*row_match)(struct pane *, struct row *, const char *);
};

struct scrollbar {
	int tickpos;
	int ticksize;
	int x; /* absolute x position on the screen */
	int y; /* absolute y position on the screen */
	int size; /* absolute size of the bar, should be > 0 */
	int focused; /* has focus or not */
	int hidden; /* is visible or not */
	int dirty; /* needs draw update */
};

struct statusbar {
	int x; /* absolute x position on the screen */
	int y; /* absolute y position on the screen */
	int width; /* absolute width of the bar */
	char *text; /* data */
	int hidden; /* is visible or not */
	int dirty; /* needs draw update */
};

struct linebar {
	int x; /* absolute x position on the screen */
	int y; /* absolute y position on the screen */
	int width; /* absolute width of the line */
	int hidden; /* is visible or not */
	int dirty; /* needs draw update */
};

/* /UI */

struct item {
	char *fields[FieldLast];
	char *line; /* allocated split line */
	/* field to match new items, if link is set match on link, else on id */
	char *matchnew;
	time_t timestamp;
	int timeok;
	int isnew;
	off_t offset; /* line offset in file for lazyload */
};

struct items {
	struct item *items;     /* array of items */
	size_t len;             /* amount of items */
	size_t cap;             /* available capacity */
};

struct feed {
	char         *name;     /* feed name */
	char         *path;     /* path to feed or NULL for stdin */
	unsigned long totalnew; /* amount of new items per feed */
	unsigned long total;    /* total items */
	FILE *fp;               /* file pointer */
};

void alldirty(void);
void cleanup(void);
void draw(void);
int getsidebarsize(void);
void markread(struct pane *, off_t, off_t, int);
void pane_draw(struct pane *);
void sighandler(int);
void updategeom(void);
void updatesidebar(void);
void urls_free(void);
int urls_isnew(const char *);
void urls_read(void);

static struct linebar linebar;
static struct statusbar statusbar;
static struct pane panes[PaneLast];
static struct scrollbar scrollbars[PaneLast]; /* each pane has a scrollbar */
static struct win win;
static size_t selpane;
/* fixed sidebar size, < 0 is automatic */
static int fixedsidebarsizes[LayoutLast] = { -1, -1, -1 };
static int layout = LayoutVertical, prevlayout = LayoutVertical;
static int onlynew = 0; /* show only new in sidebar */
static int usemouse = 1; /* use xterm mouse tracking */

static struct termios tsave; /* terminal state at startup */
static struct termios tcur;
static int devnullfd;
static int istermsetup, needcleanup;

static struct feed *feeds;
static struct feed *curfeed;
static size_t nfeeds; /* amount of feeds */
static time_t comparetime;
static char *urlfile, **urls;
static size_t nurls;

volatile sig_atomic_t sigstate = 0;

static char *plumbercmd = "/home/cjg/bin/openurlsfeed.sh"; /* env variable: $SFEED_PLUMBER */
static char *pipercmd = "sfeed_content"; /* env variable: $SFEED_PIPER */
static char *yankercmd = "xclip -r"; /* env variable: $SFEED_YANKER */
static char *markreadcmd = "sfeed_markread read"; /* env variable: $SFEED_MARK_READ */
static char *markunreadcmd = "sfeed_markread unread"; /* env variable: $SFEED_MARK_UNREAD */
static char *cmdenv; /* env variable: $SFEED_AUTOCMD */
static int plumberia = 0; /* env variable: $SFEED_PLUMBER_INTERACTIVE */
static int piperia = 1; /* env variable: $SFEED_PIPER_INTERACTIVE */
static int yankeria = 0; /* env variable: $SFEED_YANKER_INTERACTIVE */
static int lazyload = 0; /* env variable: $SFEED_LAZYLOAD */

int
ttywritef(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vfprintf(stdout, fmt, ap);
	va_end(ap);
	fflush(stdout);

	return n;
}

int
ttywrite(const char *s)
{
	if (!s)
		return 0; /* for tparm() returning NULL */
	return write(1, s, strlen(s));
}

/* hint for compilers and static analyzers that a function exits */
#ifndef __dead
#define __dead
#endif

/* print to stderr, call cleanup() and _exit(). */
__dead void
die(const char *fmt, ...)
{
	va_list ap;
	int saved_errno;

	saved_errno = errno;
	cleanup();

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (saved_errno)
		fprintf(stderr, ": %s", strerror(saved_errno));
	fflush(stderr);
	write(2, "\n", 1);

	_exit(1);
}

void *
erealloc(void *ptr, size_t size)
{
	void *p;

	if (!(p = realloc(ptr, size)))
		die("realloc");
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc");
	return p;
}

char *
estrdup(const char *s)
{
	char *p;

	if (!(p = strdup(s)))
		die("strdup");
	return p;
}

/* wrapper for tparm which allows NULL parameter for str. */
char *
tparmnull(const char *str, long p1, long p2, long p3, long p4, long p5,
          long p6, long p7, long p8, long p9)
{
	if (!str)
		return NULL;
	return tparm(str, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

/* strcasestr() included for portability */
#undef strcasestr
char *
strcasestr(const char *h, const char *n)
{
	size_t i;

	if (!n[0])
		return (char *)h;

	for (; *h; ++h) {
		for (i = 0; n[i] && tolower((unsigned char)n[i]) ==
		            tolower((unsigned char)h[i]); ++i)
			;
		if (n[i] == '\0')
			return (char *)h;
	}

	return NULL;
}

/* Splits fields in the line buffer by replacing TAB separators with NUL ('\0')
   terminators and assign these fields as pointers. If there are less fields
   than expected then the field is an empty string constant. */
void
parseline(char *line, char *fields[FieldLast])
{
	char *prev, *s;
	size_t i;

	for (prev = line, i = 0;
	    (s = strchr(prev, '\t')) && i < FieldLast - 1;
	    i++) {
		*s = '\0';
		fields[i] = prev;
		prev = s + 1;
	}
	fields[i++] = prev;
	/* make non-parsed fields empty. */
	for (; i < FieldLast; i++)
		fields[i] = "";
}

/* Parse time to time_t, assumes time_t is signed, ignores fractions. */
int
strtotime(const char *s, time_t *t)
{
	long long l;
	char *e;

	errno = 0;
	l = strtoll(s, &e, 10);
	if (errno || *s == '\0' || *e)
		return -1;
	/* NOTE: assumes time_t is 64-bit on 64-bit platforms:
	         long long (at least 32-bit) to time_t. */
	if (t)
		*t = (time_t)l;

	return 0;
}

size_t
colw(const char *s)
{
	wchar_t wc;
	size_t col = 0, i, slen;
	int inc, rl, w;

	slen = strlen(s);
	for (i = 0; i < slen; i += inc) {
		inc = 1; /* next byte */
		if ((unsigned char)s[i] < 32) {
			continue;
		} else if ((unsigned char)s[i] >= 127) {
			rl = mbtowc(&wc, &s[i], slen - i < 4 ? slen - i : 4);
			inc = rl;
			if (rl < 0) {
				mbtowc(NULL, NULL, 0); /* reset state */
				inc = 1; /* invalid, seek next byte */
				w = 1; /* replacement char is one width */
			} else if ((w = wcwidth(wc)) == -1) {
				w = 1;
			}
			col += w;
		} else {
			col++;
		}
	}
	return col;
}

/* Format `len' columns of characters. If string is shorter pad the rest
   with characters `pad`. */
int
utf8pad(char *buf, size_t bufsiz, const char *s, size_t len, int pad)
{
	wchar_t wc;
	size_t col = 0, i, slen, siz = 0;
	int inc, rl, w;

	if (!bufsiz)
		return -1;
	if (!len) {
		buf[0] = '\0';
		return 0;
	}

	slen = strlen(s);
	for (i = 0; i < slen; i += inc) {
		inc = 1; /* next byte */
		if ((unsigned char)s[i] < 32)
			continue;

		rl = mbtowc(&wc, &s[i], slen - i < 4 ? slen - i : 4);
		inc = rl;
		if (rl < 0) {
			mbtowc(NULL, NULL, 0); /* reset state */
			inc = 1; /* invalid, seek next byte */
			w = 1; /* replacement char is one width */
		} else if ((w = wcwidth(wc)) == -1) {
		        w = 1;
                }

		if (col + w > len || (col + w == len && s[i + inc])) {
			if (siz + 4 >= bufsiz)
				return -1;
			memcpy(&buf[siz], PAD_TRUNCATE_SYMBOL, sizeof(PAD_TRUNCATE_SYMBOL) - 1);
			siz += sizeof(PAD_TRUNCATE_SYMBOL) - 1;
			buf[siz] = '\0';
			col++;
			break;
		} else if (rl < 0) {
			if (siz + 4 >= bufsiz)
				return -1;
			memcpy(&buf[siz], UTF_INVALID_SYMBOL, sizeof(UTF_INVALID_SYMBOL) - 1);
			siz += sizeof(UTF_INVALID_SYMBOL) - 1;
			buf[siz] = '\0';
			col++;
			continue;
		}
		if (siz + inc + 1 >= bufsiz)
			return -1;
		memcpy(&buf[siz], &s[i], inc);
		siz += inc;
		buf[siz] = '\0';
		col += w;
	}

	len -= col;
	if (siz + len + 1 >= bufsiz)
		return -1;
	memset(&buf[siz], pad, len);
	siz += len;
	buf[siz] = '\0';

	return 0;
}

/* print `len' columns of characters. If string is shorter pad the rest with
 * characters `pad`. */
void
printutf8pad(FILE *fp, const char *s, size_t len, int pad)
{
	wchar_t wc;
	size_t col = 0, i, slen;
	int inc, rl, w;

	if (!len)
		return;

	slen = strlen(s);
	for (i = 0; i < slen; i += inc) {
		inc = 1; /* next byte */
		if ((unsigned char)s[i] < 32) {
			continue; /* skip control characters */
		} else if ((unsigned char)s[i] >= 127) {
			rl = mbtowc(&wc, s + i, slen - i < 4 ? slen - i : 4);
			inc = rl;
			if (rl < 0) {
				mbtowc(NULL, NULL, 0); /* reset state */
				inc = 1; /* invalid, seek next byte */
				w = 1; /* replacement char is one width */
			} else if ((w = wcwidth(wc)) == -1) {
				w = 1;
			}

			if (col + w > len || (col + w == len && s[i + inc])) {
				fputs("\xe2\x80\xa6", fp); /* ellipsis */
				col++;
				break;
			} else if (rl < 0) {
				fputs("\xef\xbf\xbd", fp); /* replacement */
				col++;
				continue;
			}
			fwrite(&s[i], 1, rl, fp);
			col += w;
		} else {
			/* optimization: simple ASCII character */
			if (col + 1 > len || (col + 1 == len && s[i + 1])) {
				fputs("\xe2\x80\xa6", fp); /* ellipsis */
				col++;
				break;
			}
			putc(s[i], fp);
			col++;
		}

	}
	for (; col < len; ++col)
		putc(pad, fp);
}

void
resetstate(void)
{
	ttywrite("\x1b""c"); /* rs1: reset title and state */
}

void
updatetitle(void)
{
	unsigned long totalnew = 0, total = 0;
	size_t i;

	for (i = 0; i < nfeeds; i++) {
		totalnew += feeds[i].totalnew;
		total += feeds[i].total;
	}
	ttywritef("\x1b]2;(%lu/%lu) - sfeed_curses\x1b\\", totalnew, total);
}

void
appmode(int on)
{
	ttywrite(tparmnull(on ? enter_ca_mode : exit_ca_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void
mousemode(int on)
{
	ttywrite(on ? "\x1b[?1000h" : "\x1b[?1000l"); /* xterm X10 mouse mode */
	ttywrite(on ? "\x1b[?1006h" : "\x1b[?1006l"); /* extended SGR mouse mode */
}

void
cursormode(int on)
{
	ttywrite(tparmnull(on ? cursor_normal : cursor_invisible, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void
cursormove(int x, int y)
{
	ttywrite(tparmnull(cursor_address, y, x, 0, 0, 0, 0, 0, 0, 0));
}

void
cursorsave(void)
{
	/* do not save the cursor if it won't be restored anyway */
	if (cursor_invisible)
		ttywrite(tparmnull(save_cursor, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void
cursorrestore(void)
{
	/* if the cursor cannot be hidden then move to a consistent position */
	if (cursor_invisible)
		ttywrite(tparmnull(restore_cursor, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	else
		cursormove(0, 0);
}

void
attrmode(int mode)
{
	switch (mode) {
	case ATTR_RESET:
		ttywrite(tparmnull(exit_attribute_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0));
		break;
	case ATTR_BOLD_ON:
		ttywrite(tparmnull(enter_bold_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0));
		break;
	case ATTR_FAINT_ON:
		ttywrite(tparmnull(enter_dim_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0));
		break;
	case ATTR_REVERSE_ON:
		ttywrite(tparmnull(enter_reverse_mode, 0, 0, 0, 0, 0, 0, 0, 0, 0));
		break;
	default:
		break;
	}
}

void
cleareol(void)
{
	ttywrite(tparmnull(clr_eol, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void
clearscreen(void)
{
	ttywrite(tparmnull(clear_screen, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

void
cleanup(void)
{
	struct sigaction sa;

	if (!needcleanup)
		return;
	needcleanup = 0;

	if (istermsetup) {
		resetstate();
		cursormode(1);
		appmode(0);
		clearscreen();

		if (usemouse)
			mousemode(0);
	}

	/* restore terminal settings */
	tcsetattr(0, TCSANOW, &tsave);

	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; /* require BSD signal semantics */
	sa.sa_handler = SIG_DFL;
	sigaction(SIGWINCH, &sa, NULL);
}

void
win_update(struct win *w, int width, int height)
{
	if (width != w->width || height != w->height)
		w->dirty = 1;
	w->width = width;
	w->height = height;
}

void
resizewin(void)
{
	struct winsize winsz;
	int width, height;

	if (ioctl(1, TIOCGWINSZ, &winsz) != -1) {
		width = winsz.ws_col > 0 ? winsz.ws_col : 80;
		height = winsz.ws_row > 0 ? winsz.ws_row : 24;
		win_update(&win, width, height);
	}
	if (win.dirty)
		alldirty();
}

void
init(void)
{
	struct sigaction sa;
	int errret = 1;

	needcleanup = 1;

	tcgetattr(0, &tsave);
	memcpy(&tcur, &tsave, sizeof(tcur));
	tcur.c_lflag &= ~(ECHO|ICANON);
	tcur.c_cc[VMIN] = 1;
	tcur.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &tcur);

	if (!istermsetup &&
	    (setupterm(NULL, 1, &errret) != OK || errret != 1)) {
		errno = 0;
		die("setupterm: terminfo database or entry for $TERM not found");
	}
	istermsetup = 1;
	resizewin();

	appmode(1);
	cursormode(0);

	if (usemouse)
		mousemode(usemouse);

	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; /* require BSD signal semantics */
	sa.sa_handler = sighandler;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGWINCH, &sa, NULL);
}

void
processexit(pid_t pid, int interactive)
{
	pid_t wpid;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART; /* require BSD signal semantics */
	sa.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sa, NULL);

	if (interactive) {
		while ((wpid = wait(NULL)) >= 0 && wpid != pid)
			;
		init();
		updatesidebar();
		updategeom();
		updatetitle();
	} else {
		sa.sa_handler = sighandler;
		sigaction(SIGINT, &sa, NULL);
	}
}

/* Pipe item line or item field to a program.
   If `field` is -1 then pipe the TSV line, else a specified field.
   if `interactive` is 1 then cleanup and restore the tty and wait on the
   process.
   if 0 then don't do that and also write stdout and stderr to /dev/null. */
void
pipeitem(const char *cmd, struct item *item, int field, int interactive)
{
	FILE *fp;
	pid_t pid;
	int i, status;

	if (interactive)
		cleanup();

	switch ((pid = fork())) {
	case -1:
		die("fork");
	case 0:
		if (!interactive) {
			dup2(devnullfd, 1);
			dup2(devnullfd, 2);
		}

		errno = 0;
		if (!(fp = popen(cmd, "w")))
			die("popen: %s", cmd);
		if (field == -1) {
			for (i = 0; i < FieldLast; i++) {
				if (i)
					putc('\t', fp);
				fputs(item->fields[i], fp);
			}
		} else {
			fputs(item->fields[field], fp);
		}
		putc('\n', fp);
		status = pclose(fp);
		status = WIFEXITED(status) ? WEXITSTATUS(status) : 127;
		_exit(status);
	default:
		processexit(pid, interactive);
	}
}

void
forkexec(char *argv[], int interactive)
{
	pid_t pid;

	if (interactive)
		cleanup();

	switch ((pid = fork())) {
	case -1:
		die("fork");
	case 0:
		if (!interactive) {
			dup2(devnullfd, 1);
			dup2(devnullfd, 2);
		}
		if (execvp(argv[0], argv) == -1)
			_exit(1);
	default:
		processexit(pid, interactive);
	}
}

struct row *
pane_row_get(struct pane *p, off_t pos)
{
	if (pos < 0 || pos >= p->nrows)
		return NULL;

	if (p->row_get)
		return p->row_get(p, pos);
	return p->rows + pos;
}

char *
pane_row_text(struct pane *p, struct row *row)
{
	/* custom formatter */
	if (p->row_format)
		return p->row_format(p, row);
	return row->text;
}

int
pane_row_match(struct pane *p, struct row *row, const char *s)
{
	if (p->row_match)
		return p->row_match(p, row, s);
	return (strcasestr(pane_row_text(p, row), s) != NULL);
}

void
pane_row_draw(struct pane *p, off_t pos, int selected)
{
	struct row *row;

	if (p->hidden || !p->width || !p->height ||
	    p->x >= win.width || p->y + (pos % p->height) >= win.height)
		return;

	row = pane_row_get(p, pos);

	cursorsave();
	cursormove(p->x, p->y + (pos % p->height));

	if (p->focused)
		THEME_ITEM_FOCUS();
	else
		THEME_ITEM_NORMAL();
	if (row && row->bold)
		THEME_ITEM_BOLD();
	if (selected)
		THEME_ITEM_SELECTED();
	if (row) {
		printutf8pad(stdout, pane_row_text(p, row), p->width, ' ');
		fflush(stdout);
	} else {
		ttywritef("%-*.*s", p->width, p->width, "");
	}

	attrmode(ATTR_RESET);
	cursorrestore();
}

void
pane_setpos(struct pane *p, off_t pos)
{
	if (pos < 0)
		pos = 0; /* clamp */
	if (!p->nrows)
		return; /* invalid */
	if (pos >= p->nrows)
		pos = p->nrows - 1; /* clamp */
	if (pos == p->pos)
		return; /* no change */

	/* is on different scroll region? mark whole pane dirty */
	if (((p->pos - (p->pos % p->height)) / p->height) !=
	    ((pos - (pos % p->height)) / p->height)) {
		p->dirty = 1;
	} else {
		/* only redraw the 2 dirty rows */
		pane_row_draw(p, p->pos, 0);
		pane_row_draw(p, pos, 1);
	}
	p->pos = pos;
}

void
pane_scrollpage(struct pane *p, int pages)
{
	off_t pos;

	if (pages < 0) {
		pos = p->pos - (-pages * p->height);
		pos -= (p->pos % p->height);
		pos += p->height - 1;
		pane_setpos(p, pos);
	} else if (pages > 0) {
		pos = p->pos + (pages * p->height);
		if ((p->pos % p->height))
			pos -= (p->pos % p->height);
		pane_setpos(p, pos);
	}
}

void
pane_scrolln(struct pane *p, int n)
{
	pane_setpos(p, p->pos + n);
}

void
pane_setfocus(struct pane *p, int on)
{
	if (p->focused != on) {
		p->focused = on;
		p->dirty = 1;
	}
}

void
pane_draw(struct pane *p)
{
	off_t pos, y;

	if (!p->dirty)
		return;
	p->dirty = 0;
	if (p->hidden || !p->width || !p->height)
		return;

	/* draw visible rows */
	pos = p->pos - (p->pos % p->height);
	for (y = 0; y < p->height; y++)
		pane_row_draw(p, y + pos, (y + pos) == p->pos);
}

void
setlayout(int n)
{
	if (layout != LayoutMonocle)
		prevlayout = layout; /* previous non-monocle layout */
	layout = n;
}

void
updategeom(void)
{
	int h, w, x = 0, y = 0;

	panes[PaneFeeds].hidden = layout == LayoutMonocle && (selpane != PaneFeeds);
	panes[PaneItems].hidden = layout == LayoutMonocle && (selpane != PaneItems);
	linebar.hidden = layout != LayoutHorizontal;

	w = win.width;
	/* always reserve space for statusbar */
	h = MAX(win.height - 1, 1);

	panes[PaneFeeds].x = x;
	panes[PaneFeeds].y = y;

	switch (layout) {
	case LayoutVertical:
		panes[PaneFeeds].width = getsidebarsize();

		x += panes[PaneFeeds].width;
		w -= panes[PaneFeeds].width;

		/* space for scrollbar if sidebar is visible */
		w--;
		x++;

		panes[PaneFeeds].height = MAX(h, 1);
		break;
	case LayoutHorizontal:
		panes[PaneFeeds].height = getsidebarsize();

		h -= panes[PaneFeeds].height;
		y += panes[PaneFeeds].height;

		linebar.x = 0;
		linebar.y = y;
		linebar.width = win.width;

		h--;
		y++;

		panes[PaneFeeds].width = MAX(w - 1, 0);
		break;
	case LayoutMonocle:
		panes[PaneFeeds].height = MAX(h, 1);
		panes[PaneFeeds].width = MAX(w - 1, 0);
		break;
	}

	panes[PaneItems].x = x;
	panes[PaneItems].y = y;
	panes[PaneItems].width = MAX(w - 1, 0);
	panes[PaneItems].height = MAX(h, 1);
	if (x >= win.width || y + 1 >= win.height)
		panes[PaneItems].hidden = 1;

	scrollbars[PaneFeeds].x = panes[PaneFeeds].x + panes[PaneFeeds].width;
	scrollbars[PaneFeeds].y = panes[PaneFeeds].y;
	scrollbars[PaneFeeds].size = panes[PaneFeeds].height;
	scrollbars[PaneFeeds].hidden = panes[PaneFeeds].hidden;

	scrollbars[PaneItems].x = panes[PaneItems].x + panes[PaneItems].width;
	scrollbars[PaneItems].y = panes[PaneItems].y;
	scrollbars[PaneItems].size = panes[PaneItems].height;
	scrollbars[PaneItems].hidden = panes[PaneItems].hidden;

	statusbar.width = win.width;
	statusbar.x = 0;
	statusbar.y = MAX(win.height - 1, 0);

	alldirty();
}

void
scrollbar_setfocus(struct scrollbar *s, int on)
{
	if (s->focused != on) {
		s->focused = on;
		s->dirty = 1;
	}
}

void
scrollbar_update(struct scrollbar *s, off_t pos, off_t nrows, int pageheight)
{
	int tickpos = 0, ticksize = 0;

	/* do not show a scrollbar if all items fit on the page */
	if (nrows > pageheight) {
		ticksize = s->size / ((double)nrows / (double)pageheight);
		if (ticksize == 0)
			ticksize = 1;

		tickpos = (pos / (double)nrows) * (double)s->size;

		/* fixup due to cell precision */
		if (pos + pageheight >= nrows ||
		    tickpos + ticksize >= s->size)
			tickpos = s->size - ticksize;
	}

	if (s->tickpos != tickpos || s->ticksize != ticksize)
		s->dirty = 1;
	s->tickpos = tickpos;
	s->ticksize = ticksize;
}

void
scrollbar_draw(struct scrollbar *s)
{
	off_t y;

	if (!s->dirty)
		return;
	s->dirty = 0;
	if (s->hidden || !s->size || s->x >= win.width || s->y >= win.height)
		return;

	cursorsave();

	/* draw bar (not tick) */
	if (s->focused)
		THEME_SCROLLBAR_FOCUS();
	else
		THEME_SCROLLBAR_NORMAL();
	for (y = 0; y < s->size; y++) {
		if (y >= s->tickpos && y < s->tickpos + s->ticksize)
			continue; /* skip tick */
		cursormove(s->x, s->y + y);
		ttywrite(SCROLLBAR_SYMBOL_BAR);
	}

	/* draw tick */
	if (s->focused)
		THEME_SCROLLBAR_TICK_FOCUS();
	else
		THEME_SCROLLBAR_TICK_NORMAL();
	for (y = s->tickpos; y < s->size && y < s->tickpos + s->ticksize; y++) {
		cursormove(s->x, s->y + y);
		ttywrite(SCROLLBAR_SYMBOL_TICK);
	}

	attrmode(ATTR_RESET);
	cursorrestore();
}

int
readch(void)
{
	unsigned char b;
	fd_set readfds;
	struct timeval tv;

	if (cmdenv && *cmdenv)
		return *(cmdenv++);

	for (;;) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		tv.tv_sec = 0;
		tv.tv_usec = 250000; /* 250ms */
		switch (select(1, &readfds, NULL, NULL, &tv)) {
		case -1:
			if (errno != EINTR)
				die("select");
			return -2; /* EINTR: like a signal */
		case 0:
			return -3; /* time-out */
		}

		switch (read(0, &b, 1)) {
		case -1: die("read");
		case 0: return EOF;
		default: return (int)b;
		}
	}
}

char *
lineeditor(void)
{
	char *input = NULL;
	size_t cap = 0, nchars = 0;
	int ch;

	for (;;) {
		if (nchars + 1 >= cap) {
			cap = cap ? cap * 2 : 32;
			input = erealloc(input, cap);
		}

		ch = readch();
		if (ch == EOF || ch == '\r' || ch == '\n') {
			input[nchars] = '\0';
			break;
		} else if (ch == '\b' || ch == 0x7f) {
			if (!nchars)
				continue;
			input[--nchars] = '\0';
			write(1, "\b \b", 3); /* back, blank, back */
			continue;
		} else if (ch >= ' ') {
			input[nchars] = ch;
			write(1, &input[nchars], 1);
			nchars++;
		} else if (ch < 0) {
			switch (sigstate) {
			case 0:
			case SIGWINCH:
				continue; /* process signals later */
			case SIGINT:
				sigstate = 0; /* exit prompt, do not quit */
			case SIGTERM:
				break; /* exit prompt and quit */
			}
			free(input);
			return NULL;
		}
	}
	return input;
}

char *
uiprompt(int x, int y, char *fmt, ...)
{
	va_list ap;
	char *input, buf[32];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	cursorsave();
	cursormove(x, y);
	THEME_INPUT_LABEL();
	ttywrite(buf);
	attrmode(ATTR_RESET);

	THEME_INPUT_NORMAL();
	cleareol();
	cursormode(1);
	cursormove(x + colw(buf) + 1, y);

	input = lineeditor();
	attrmode(ATTR_RESET);

	cursormode(0);
	cursorrestore();

	return input;
}

void
linebar_draw(struct linebar *b)
{
	int i;

	if (!b->dirty)
		return;
	b->dirty = 0;
	if (b->hidden || !b->width)
		return;

	cursorsave();
	cursormove(b->x, b->y);
	THEME_LINEBAR();
	for (i = 0; i < b->width - 1; i++)
		ttywrite(LINEBAR_SYMBOL_BAR);
	ttywrite(LINEBAR_SYMBOL_RIGHT);
	attrmode(ATTR_RESET);
	cursorrestore();
}

void
statusbar_draw(struct statusbar *s)
{
	if (!s->dirty)
		return;
	s->dirty = 0;
	if (s->hidden || !s->width || s->x >= win.width || s->y >= win.height)
		return;

	cursorsave();
	cursormove(s->x, s->y);
	THEME_STATUSBAR();
	/* terminals without xenl (eat newline glitch) mess up scrolling when
	   using the last cell on the last line on the screen. */
	printutf8pad(stdout, s->text, s->width - (!eat_newline_glitch), ' ');
	fflush(stdout);
	attrmode(ATTR_RESET);
	cursorrestore();
}

void
statusbar_update(struct statusbar *s, const char *text)
{
	if (s->text && !strcmp(s->text, text))
		return;

	free(s->text);
	s->text = estrdup(text);
	s->dirty = 1;
}

/* Line to item, modifies and splits line in-place. */
int
linetoitem(char *line, struct item *item)
{
	char *fields[FieldLast];
	time_t parsedtime;

	item->line = line;
	parseline(line, fields);
	memcpy(item->fields, fields, sizeof(fields));
	if (urlfile)
		item->matchnew = estrdup(fields[fields[FieldLink][0] ? FieldLink : FieldId]);
	else
		item->matchnew = NULL;

	parsedtime = 0;
	if (!strtotime(fields[FieldUnixTimestamp], &parsedtime)) {
		item->timestamp = parsedtime;
		item->timeok = 1;
	} else {
		item->timestamp = 0;
		item->timeok = 0;
	}

	return 0;
}

void
feed_items_free(struct items *items)
{
	size_t i;

	for (i = 0; i < items->len; i++) {
		free(items->items[i].line);
		free(items->items[i].matchnew);
	}
	free(items->items);
	items->items = NULL;
	items->len = 0;
	items->cap = 0;
}

void
feed_items_get(struct feed *f, FILE *fp, struct items *itemsret)
{
	struct item *item, *items = NULL;
	char *line = NULL;
	size_t cap, i, linesize = 0, nitems;
	ssize_t linelen, n;
	off_t offset;

	cap = nitems = 0;
	offset = 0;
	for (i = 0; ; i++) {
		if (i + 1 >= cap) {
			cap = cap ? cap * 2 : 16;
			items = erealloc(items, cap * sizeof(struct item));
		}
		if ((n = linelen = getline(&line, &linesize, fp)) > 0) {
			item = &items[i];

			item->offset = offset;
			offset += linelen;

			if (line[linelen - 1] == '\n')
				line[--linelen] = '\0';

			if (lazyload && f->path) {
				linetoitem(line, item);

				/* data is ignored here, will be lazy-loaded later. */
				item->line = NULL;
				memset(item->fields, 0, sizeof(item->fields));
			} else {
				linetoitem(estrdup(line), item);
			}

			nitems++;
		}
		if (ferror(fp))
			die("getline: %s", f->name);
		if (n <= 0 || feof(fp))
			break;
	}
	itemsret->cap = cap;
	itemsret->items = items;
	itemsret->len = nitems;
	free(line);
}

void
updatenewitems(struct feed *f)
{
	struct pane *p;
	struct row *row;
	struct item *item;
	size_t i;

	p = &panes[PaneItems];
	f->totalnew = 0;
	for (i = 0; i < p->nrows; i++) {
		row = &(p->rows[i]); /* do not use pane_row_get */
		item = row->data;
		if (urlfile)
			item->isnew = urls_isnew(item->matchnew);
		else
			item->isnew = (item->timeok && item->timestamp >= comparetime);
		row->bold = item->isnew;
		f->totalnew += item->isnew;
	}
	f->total = p->nrows;
}

void
feed_load(struct feed *f, FILE *fp)
{
	/* static, reuse local buffers */
	static struct items items;
	struct pane *p;
	size_t i;

	feed_items_free(&items);
	feed_items_get(f, fp, &items);
	p = &panes[PaneItems];
	p->pos = 0;
	p->nrows = items.len;
	free(p->rows);
	p->rows = ecalloc(sizeof(p->rows[0]), items.len + 1);
	for (i = 0; i < items.len; i++)
		p->rows[i].data = &(items.items[i]); /* do not use pane_row_get */

	updatenewitems(f);

	p->dirty = 1;
}

void
feed_count(struct feed *f, FILE *fp)
{
	char *fields[FieldLast];
	char *line = NULL;
	size_t linesize = 0;
	ssize_t linelen;
	time_t parsedtime;

	f->totalnew = f->total = 0;
	while ((linelen = getline(&line, &linesize, fp)) > 0) {
		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';
		parseline(line, fields);

		if (urlfile) {
			f->totalnew += urls_isnew(fields[fields[FieldLink][0] ? FieldLink : FieldId]);
		} else {
			parsedtime = 0;
			if (!strtotime(fields[FieldUnixTimestamp], &parsedtime))
				f->totalnew += (parsedtime >= comparetime);
		}
		f->total++;
	}
	if (ferror(fp))
		die("getline: %s", f->name);
	free(line);
}

void
feed_setenv(struct feed *f)
{
	if (f && f->path)
		setenv("SFEED_FEED_PATH", f->path, 1);
	else
		unsetenv("SFEED_FEED_PATH");
}

/* Change feed, have one file open, reopen file if needed. */
void
feeds_set(struct feed *f)
{
	if (curfeed) {
		if (curfeed->path && curfeed->fp) {
			fclose(curfeed->fp);
			curfeed->fp = NULL;
		}
	}

	if (f && f->path) {
		if (!f->fp && !(f->fp = fopen(f->path, "rb")))
			die("fopen: %s", f->path);
	}

	feed_setenv(f);

	curfeed = f;
}

void
feeds_load(struct feed *feeds, size_t nfeeds)
{
	struct feed *f;
	size_t i;

	if ((comparetime = time(NULL)) == -1)
		die("time");
	/* 1 day is old news */
	comparetime -= 86400;

	for (i = 0; i < nfeeds; i++) {
		f = &feeds[i];

		if (f->path) {
			if (f->fp) {
				if (fseek(f->fp, 0, SEEK_SET))
					die("fseek: %s", f->path);
			} else {
				if (!(f->fp = fopen(f->path, "rb")))
					die("fopen: %s", f->path);
			}
		}
		if (!f->fp) {
			/* reading from stdin, just recount new */
			if (f == curfeed)
				updatenewitems(f);
			continue;
		}

		/* load first items, because of first selection or stdin. */
		if (f == curfeed) {
			feed_load(f, f->fp);
		} else {
			feed_count(f, f->fp);
			if (f->path && f->fp) {
				fclose(f->fp);
				f->fp = NULL;
			}
		}
	}
}

/* find row position of the feed if visible, else return -1 */
off_t
feeds_row_get(struct pane *p, struct feed *f)
{
	struct row *row;
	struct feed *fr;
	off_t pos;

	for (pos = 0; pos < p->nrows; pos++) {
		if (!(row = pane_row_get(p, pos)))
			continue;
		fr = row->data;
		if (!strcmp(fr->name, f->name))
			return pos;
	}
	return -1;
}

void
feeds_reloadall(void)
{
	struct pane *p;
	struct feed *f = NULL;
	struct row *row;
	off_t pos;

	p = &panes[PaneFeeds];
	if ((row = pane_row_get(p, p->pos)))
		f = row->data;

	pos = panes[PaneItems].pos; /* store numeric item position */
	feeds_set(curfeed); /* close and reopen feed if possible */
	urls_read();
	feeds_load(feeds, nfeeds);
	urls_free();
	/* restore numeric item position */
	pane_setpos(&panes[PaneItems], pos);
	updatesidebar();
	updatetitle();

	/* try to find the same feed in the pane */
	if (f && (pos = feeds_row_get(p, f)) != -1)
		pane_setpos(p, pos);
	else
		pane_setpos(p, 0);
}

void
feed_open_selected(struct pane *p)
{
	struct feed *f;
	struct row *row;

	if (!(row = pane_row_get(p, p->pos)))
		return;
	f = row->data;
	feeds_set(f);
	urls_read();
	if (f->fp)
		feed_load(f, f->fp);
	urls_free();
	/* redraw row: counts could be changed */
	updatesidebar();
	updatetitle();

	if (layout == LayoutMonocle) {
		selpane = PaneItems;
		updategeom();
	}
}

void
feed_plumb_selected_item(struct pane *p, int field)
{
	struct row *row;
	struct item *item;

	if (!(row = pane_row_get(p, p->pos)))
		return;
	item = row->data;
	markread(p, p->pos, p->pos, 1);
	forkexec((char *[]) { plumbercmd, item->fields[field], NULL }, plumberia);
}

void
feed_pipe_selected_item(struct pane *p)
{
	struct row *row;
	struct item *item;

	if (!(row = pane_row_get(p, p->pos)))
		return;
	item = row->data;
	markread(p, p->pos, p->pos, 1);
	pipeitem(pipercmd, item, -1, piperia);
}

void
feed_yank_selected_item(struct pane *p, int field)
{
	struct row *row;
	struct item *item;

	if (!(row = pane_row_get(p, p->pos)))
		return;
	item = row->data;
	pipeitem(yankercmd, item, field, yankeria);
}

/* calculate optimal (default) size */
int
getsidebarsizedefault(void)
{
	struct feed *feed;
	size_t i;
	int len, size;

	switch (layout) {
	case LayoutVertical:
		for (i = 0, size = 0; i < nfeeds; i++) {
			feed = &feeds[i];
			len = snprintf(NULL, 0, " (%lu/%lu)",
			               feed->totalnew, feed->total) +
				       colw(feed->name);
			if (len > size)
				size = len;

			if (onlynew && feed->totalnew == 0)
				continue;
		}
		return MAX(MIN(win.width - 1, size), 0);
	case LayoutHorizontal:
		for (i = 0, size = 0; i < nfeeds; i++) {
			feed = &feeds[i];
			if (onlynew && feed->totalnew == 0)
				continue;
			size++;
		}
		return MAX(MIN((win.height - 1) / 2, size), 1);
	}
	return 0;
}

int
getsidebarsize(void)
{
	int size;

	if ((size = fixedsidebarsizes[layout]) < 0)
		size = getsidebarsizedefault();
	return size;
}

void
adjustsidebarsize(int n)
{
	int size;

	if ((size = fixedsidebarsizes[layout]) < 0)
		size = getsidebarsizedefault();
	if (n > 0) {
		if ((layout == LayoutVertical && size + 1 < win.width) ||
		    (layout == LayoutHorizontal && size + 1 < win.height))
			size++;
	} else if (n < 0) {
		if ((layout == LayoutVertical && size > 0) ||
		    (layout == LayoutHorizontal && size > 1))
			size--;
	}

	if (size != fixedsidebarsizes[layout]) {
		fixedsidebarsizes[layout] = size;
		updategeom();
	}
}

void
updatesidebar(void)
{
	struct pane *p;
	struct row *row;
	struct feed *feed;
	size_t i, nrows;
	int oldvalue = 0, newvalue = 0;

	p = &panes[PaneFeeds];
	if (!p->rows)
		p->rows = ecalloc(sizeof(p->rows[0]), nfeeds + 1);

	switch (layout) {
	case LayoutVertical:
		oldvalue = p->width;
		newvalue = getsidebarsize();
		p->width = newvalue;
		break;
	case LayoutHorizontal:
		oldvalue = p->height;
		newvalue = getsidebarsize();
		p->height = newvalue;
		break;
	}

	nrows = 0;
	for (i = 0; i < nfeeds; i++) {
		feed = &feeds[i];

		row = &(p->rows[nrows]);
		row->bold = (feed->totalnew > 0);
		row->data = feed;

		if (onlynew && feed->totalnew == 0)
			continue;

		nrows++;
	}
	p->nrows = nrows;

	if (oldvalue != newvalue)
		updategeom();
	else
		p->dirty = 1;

	if (!p->nrows)
		p->pos = 0;
	else if (p->pos >= p->nrows)
		p->pos = p->nrows - 1;
}

void
sighandler(int signo)
{
	switch (signo) {
	case SIGHUP:
	case SIGINT:
	case SIGTERM:
	case SIGWINCH:
		/* SIGTERM is more important, do not override it */
		if (sigstate != SIGTERM)
			sigstate = signo;
		break;
	}
}

void
alldirty(void)
{
	win.dirty = 1;
	panes[PaneFeeds].dirty = 1;
	panes[PaneItems].dirty = 1;
	scrollbars[PaneFeeds].dirty = 1;
	scrollbars[PaneItems].dirty = 1;
	linebar.dirty = 1;
	statusbar.dirty = 1;
}

void
draw(void)
{
	struct row *row;
	struct item *item;
	size_t i;

	if (win.dirty)
		win.dirty = 0;

	for (i = 0; i < LEN(panes); i++) {
		pane_setfocus(&panes[i], i == selpane);
		pane_draw(&panes[i]);

		/* each pane has a scrollbar */
		scrollbar_setfocus(&scrollbars[i], i == selpane);
		scrollbar_update(&scrollbars[i],
		                 panes[i].pos - (panes[i].pos % panes[i].height),
		                 panes[i].nrows, panes[i].height);
		scrollbar_draw(&scrollbars[i]);
	}

	linebar_draw(&linebar);

	/* if item selection text changed then update the status text */
	if ((row = pane_row_get(&panes[PaneItems], panes[PaneItems].pos))) {
		item = row->data;
		statusbar_update(&statusbar, item->fields[FieldLink]);
	} else {
		statusbar_update(&statusbar, "");
	}
	statusbar_draw(&statusbar);
}

void
mousereport(int button, int release, int keymask, int x, int y)
{
	struct pane *p;
	size_t i;
	int changedpane, dblclick, pos;

	if (!usemouse || release || button == -1)
		return;

	for (i = 0; i < LEN(panes); i++) {
		p = &panes[i];
		if (p->hidden || !p->width || !p->height)
			continue;

		/* these button actions are done regardless of the position */
		switch (button) {
		case 7: /* side-button: backward */
			if (selpane == PaneFeeds)
				return;
			selpane = PaneFeeds;
			if (layout == LayoutMonocle)
				updategeom();
			return;
		case 8: /* side-button: forward */
			if (selpane == PaneItems)
				return;
			selpane = PaneItems;
			if (layout == LayoutMonocle)
				updategeom();
			return;
		}

		/* check if mouse position is in pane or in its scrollbar */
		if (!(x >= p->x && x < p->x + p->width + (!scrollbars[i].hidden) &&
		      y >= p->y && y < p->y + p->height))
			continue;

		changedpane = (selpane != i);
		selpane = i;
		/* relative position on screen */
		pos = y - p->y + p->pos - (p->pos % p->height);
		dblclick = (pos == p->pos); /* clicking the same row twice */

		switch (button) {
		case 0: /* left-click */
			if (!p->nrows || pos >= p->nrows)
				break;
			pane_setpos(p, pos);
			if (i == PaneFeeds)
				feed_open_selected(&panes[PaneFeeds]);
			else if (i == PaneItems && dblclick && !changedpane)
				feed_plumb_selected_item(&panes[PaneItems], FieldLink);
			break;
		case 2: /* right-click */
			if (!p->nrows || pos >= p->nrows)
				break;
			pane_setpos(p, pos);
			if (i == PaneItems)
				feed_pipe_selected_item(&panes[PaneItems]);
			break;
		case 3: /* scroll up */
		case 4: /* scroll down */
			pane_scrollpage(p, button == 3 ? -1 : +1);
			break;
		}
		return; /* do not bubble events */
	}
}

/* Custom formatter for feed row. */
char *
feed_row_format(struct pane *p, struct row *row)
{
	/* static, reuse local buffers */
	static char *bufw, *text;
	static size_t bufwsize, textsize;
	struct feed *feed;
	size_t needsize;
	char counts[128];
	int len, w;

	feed = row->data;

	/* align counts to the right and pad the rest with spaces */
	len = snprintf(counts, sizeof(counts), "(%lu/%lu)",
	               feed->totalnew, feed->total);
	if (len > p->width)
		w = p->width;
	else
		w = p->width - len;

	needsize = (w + 1) * 4;
	if (needsize > bufwsize) {
		bufw = erealloc(bufw, needsize);
		bufwsize = needsize;
	}

	needsize = bufwsize + sizeof(counts) + 1;
	if (needsize > textsize) {
		text = erealloc(text, needsize);
		textsize = needsize;
	}

	if (utf8pad(bufw, bufwsize, feed->name, w, ' ') != -1)
		snprintf(text, textsize, "%s%s", bufw, counts);
	else
		text[0] = '\0';

	return text;
}

int
feed_row_match(struct pane *p, struct row *row, const char *s)
{
	struct feed *feed;

	feed = row->data;

	return (strcasestr(feed->name, s) != NULL);
}

struct row *
item_row_get(struct pane *p, off_t pos)
{
	struct row *itemrow;
	struct item *item;
	struct feed *f;
	char *line = NULL;
	size_t linesize = 0;
	ssize_t linelen;

	itemrow = p->rows + pos;
	item = itemrow->data;

	f = curfeed;
	if (f && f->path && f->fp && !item->line) {
		if (fseek(f->fp, item->offset, SEEK_SET))
			die("fseek: %s", f->path);

		if ((linelen = getline(&line, &linesize, f->fp)) <= 0) {
			if (ferror(f->fp))
				die("getline: %s", f->path);
			return NULL;
		}

		if (line[linelen - 1] == '\n')
			line[--linelen] = '\0';

		linetoitem(estrdup(line), item);
		free(line);

		itemrow->data = item;
	}
	return itemrow;
}

/* Custom formatter for item row. */
char *
item_row_format(struct pane *p, struct row *row)
{
	/* static, reuse local buffers */
	static char *text;
	static size_t textsize;
	struct item *item;
	struct tm tm;
	size_t needsize;

	item = row->data;

	needsize = strlen(item->fields[FieldTitle]) + 21;
	if (needsize > textsize) {
		text = erealloc(text, needsize);
		textsize = needsize;
	}

	if (item->timeok && localtime_r(&(item->timestamp), &tm)) {
		snprintf(text, textsize, "%c %04d-%02d-%02d %02d:%02d %s",
		         item->fields[FieldEnclosure][0] ? '@' : ' ',
		         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		         tm.tm_hour, tm.tm_min, item->fields[FieldTitle]);
	} else {
		snprintf(text, textsize, "%c                  %s",
		         item->fields[FieldEnclosure][0] ? '@' : ' ',
		         item->fields[FieldTitle]);
	}

	return text;
}

void
markread(struct pane *p, off_t from, off_t to, int isread)
{
	struct row *row;
	struct item *item;
	FILE *fp;
	off_t i;
	const char *cmd;
	int isnew = !isread, pid, wpid, status, visstart;

	if (!urlfile || !p->nrows)
		return;

	cmd = isread ? markreadcmd : markunreadcmd;

	switch ((pid = fork())) {
	case -1:
		die("fork");
	case 0:
		dup2(devnullfd, 1);
		dup2(devnullfd, 2);

		errno = 0;
		if (!(fp = popen(cmd, "w")))
			die("popen: %s", cmd);

		for (i = from; i <= to && i < p->nrows; i++) {
			/* do not use pane_row_get: no need for lazyload */
			row = &(p->rows[i]);
			item = row->data;
			if (item->isnew != isnew) {
				fputs(item->matchnew, fp);
				putc('\n', fp);
			}
		}
		status = pclose(fp);
		status = WIFEXITED(status) ? WEXITSTATUS(status) : 127;
		_exit(status);
	default:
		while ((wpid = wait(&status)) >= 0 && wpid != pid)
			;

		/* fail: exit statuscode was non-zero */
		if (status)
			break;

		visstart = p->pos - (p->pos % p->height); /* visible start */
		for (i = from; i <= to && i < p->nrows; i++) {
			row = &(p->rows[i]);
			item = row->data;
			if (item->isnew == isnew)
				continue;

			row->bold = item->isnew = isnew;
			curfeed->totalnew += isnew ? 1 : -1;

			/* draw if visible on screen */
			if (i >= visstart && i < visstart + p->height)
				pane_row_draw(p, i, i == p->pos);
		}
		updatesidebar();
		updatetitle();
	}
}

int
urls_cmp(const void *v1, const void *v2)
{
	return strcmp(*((char **)v1), *((char **)v2));
}

int
urls_isnew(const char *url)
{
	return bsearch(&url, urls, nurls, sizeof(char *), urls_cmp) == NULL;
}

void
urls_free(void)
{
	while (nurls > 0)
		free(urls[--nurls]);
	free(urls);
	urls = NULL;
	nurls = 0;
}

void
urls_read(void)
{
	FILE *fp;
	char *line = NULL;
	size_t linesiz = 0, cap = 0;
	ssize_t n;

	urls_free();

	if (!urlfile)
		return;
	if (!(fp = fopen(urlfile, "rb")))
		die("fopen: %s", urlfile);

	while ((n = getline(&line, &linesiz, fp)) > 0) {
		if (line[n - 1] == '\n')
			line[--n] = '\0';
		if (nurls + 1 >= cap) {
			cap = cap ? cap * 2 : 16;
			urls = erealloc(urls, cap * sizeof(char *));
		}
		urls[nurls++] = estrdup(line);
	}
	if (ferror(fp))
		die("getline: %s", urlfile);
	fclose(fp);
	free(line);

	qsort(urls, nurls, sizeof(char *), urls_cmp);
}

int
main(int argc, char *argv[])
{
	struct pane *p;
	struct feed *f;
	struct row *row;
	size_t i;
	char *name, *tmp;
	char *search = NULL; /* search text */
	int button, ch, fd, keymask, release, x, y;
	off_t pos;

#ifdef __OpenBSD__
	if (pledge("stdio rpath tty proc exec", NULL) == -1)
		die("pledge");
#endif

	setlocale(LC_CTYPE, "");

	if ((tmp = getenv("SFEED_PLUMBER")))
		plumbercmd = tmp;
	if ((tmp = getenv("SFEED_PIPER")))
		pipercmd = tmp;
	if ((tmp = getenv("SFEED_YANKER")))
		yankercmd = tmp;
	if ((tmp = getenv("SFEED_PLUMBER_INTERACTIVE")))
		plumberia = !strcmp(tmp, "1");
	if ((tmp = getenv("SFEED_PIPER_INTERACTIVE")))
		piperia = !strcmp(tmp, "1");
	if ((tmp = getenv("SFEED_YANKER_INTERACTIVE")))
		yankeria = !strcmp(tmp, "1");
	if ((tmp = getenv("SFEED_MARK_READ")))
		markreadcmd = tmp;
	if ((tmp = getenv("SFEED_MARK_UNREAD")))
		markunreadcmd = tmp;
	if ((tmp = getenv("SFEED_LAZYLOAD")))
		lazyload = !strcmp(tmp, "1");
	urlfile = getenv("SFEED_URL_FILE"); /* can be NULL */
	cmdenv = getenv("SFEED_AUTOCMD"); /* can be NULL */

	setlayout(argc <= 1 ? LayoutMonocle : LayoutVertical);
	selpane = layout == LayoutMonocle ? PaneItems : PaneFeeds;

	panes[PaneFeeds].row_format = feed_row_format;
	panes[PaneFeeds].row_match = feed_row_match;
	panes[PaneItems].row_format = item_row_format;
	if (lazyload)
		panes[PaneItems].row_get = item_row_get;

	feeds = ecalloc(argc, sizeof(struct feed));
	if (argc == 1) {
		nfeeds = 1;
		f = &feeds[0];
		f->name = "stdin";
		if (!(f->fp = fdopen(0, "rb")))
			die("fdopen");
	} else {
		for (i = 1; i < argc; i++) {
			f = &feeds[i - 1];
			f->path = argv[i];
			name = ((name = strrchr(argv[i], '/'))) ? name + 1 : argv[i];
			f->name = name;
		}
		nfeeds = argc - 1;
	}
	feeds_set(&feeds[0]);
	urls_read();
	feeds_load(feeds, nfeeds);
	urls_free();

	if (!isatty(0)) {
		if ((fd = open("/dev/tty", O_RDONLY)) == -1)
			die("open: /dev/tty");
		if (dup2(fd, 0) == -1)
			die("dup2(%d, 0): /dev/tty -> stdin", fd);
		close(fd);
	}
	if (argc == 1)
		feeds[0].fp = NULL;

	if ((devnullfd = open("/dev/null", O_WRONLY)) == -1)
		die("open: /dev/null");

	init();
	updatesidebar();
	updategeom();
	updatetitle();
	draw();

	while (1) {
		if ((ch = readch()) < 0)
			goto event;
		switch (ch) {
		case '\x1b':
			if ((ch = readch()) < 0)
				goto event;
			if (ch != '[' && ch != 'O')
				continue; /* unhandled */
			if ((ch = readch()) < 0)
				goto event;
			switch (ch) {
			case 'M': /* mouse: X10 encoding */
				if ((ch = readch()) < 0)
					goto event;
				button = ch - 32;
				if ((ch = readch()) < 0)
					goto event;
				x = ch - 32;
				if ((ch = readch()) < 0)
					goto event;
				y = ch - 32;

				keymask = button & (4 | 8 | 16); /* shift, meta, ctrl */
				button &= ~keymask; /* unset key mask */

				/* button numbers (0 - 2) encoded in lowest 2 bits
				   release does not indicate which button (so set to 0).
				   Handle extended buttons like scrollwheels
				   and side-buttons by each range. */
				release = 0;
				if (button == 3) {
					button = -1;
					release = 1;
				} else if (button >= 128) {
					button -= 121;
				} else if (button >= 64) {
					button -= 61;
				}
				mousereport(button, release, keymask, x - 1, y - 1);
				break;
			case '<': /* mouse: SGR encoding */
				for (button = 0; ; button *= 10, button += ch - '0') {
					if ((ch = readch()) < 0)
						goto event;
					else if (ch == ';')
						break;
				}
				for (x = 0; ; x *= 10, x += ch - '0') {
					if ((ch = readch()) < 0)
						goto event;
					else if (ch == ';')
						break;
				}
				for (y = 0; ; y *= 10, y += ch - '0') {
					if ((ch = readch()) < 0)
						goto event;
					else if (ch == 'm' || ch == 'M')
						break; /* release or press */
				}
				release = ch == 'm';
				keymask = button & (4 | 8 | 16); /* shift, meta, ctrl */
				button &= ~keymask; /* unset key mask */

				if (button >= 128)
					button -= 121;
				else if (button >= 64)
					button -= 61;

				mousereport(button, release, keymask, x - 1, y - 1);
				break;
			case 'A': goto keyup;    /* arrow up */
			case 'B': goto keydown;  /* arrow down */
			case 'C': goto keyright; /* arrow left */
			case 'D': goto keyleft;  /* arrow right */
			case 'F': goto endpos;   /* end */
			case 'H': goto startpos; /* home */
			case '4': /* end */
				if ((ch = readch()) < 0)
					goto event;
				if (ch == '~')
					goto endpos;
				continue;
			case '5': /* page up */
				if ((ch = readch()) < 0)
					goto event;
				if (ch == '~')
					goto prevpage;
				continue;
			case '6': /* page down */
				if ((ch = readch()) < 0)
					goto event;
				if (ch == '~')
					goto nextpage;
				continue;
			}
			break;
keyup:
		case 'k':
		case 'K':
			pane_scrolln(&panes[selpane], -1);
			if (ch == 'K')
				goto openitem;
			break;
keydown:
		case 'j':
		case 'J':
			pane_scrolln(&panes[selpane], +1);
			if (ch == 'J')
				goto openitem;
			break;
keyleft:
		case 'h':
			if (selpane == PaneFeeds)
				break;
			selpane = PaneFeeds;
			if (layout == LayoutMonocle)
				updategeom();
			break;
keyright:
		case 'l':
			if (selpane == PaneItems)
				break;
			selpane = PaneItems;
			if (layout == LayoutMonocle)
				updategeom();
			break;
		case '\t':
			selpane = selpane == PaneFeeds ? PaneItems : PaneFeeds;
			if (layout == LayoutMonocle)
				updategeom();
			break;
startpos:
		case 'g':
			pane_setpos(&panes[selpane], 0);
			break;
endpos:
		case 'G':
			p = &panes[selpane];
			if (p->nrows)
				pane_setpos(p, p->nrows - 1);
			break;
prevpage:
		case 2: /* ^B */
			pane_scrollpage(&panes[selpane], -1);
			break;
nextpage:
		case ' ':
		case 6: /* ^F */
			pane_scrollpage(&panes[selpane], +1);
			break;
		case '[':
		case ']':
			pane_scrolln(&panes[PaneFeeds], ch == '[' ? -1 : +1);
			feed_open_selected(&panes[PaneFeeds]);
			break;
		case '/': /* new search (forward) */
		case '?': /* new search (backward) */
		case 'n': /* search again (forward) */
		case 'N': /* search again (backward) */
			p = &panes[selpane];

			/* prompt for new input */
			if (ch == '?' || ch == '/') {
				tmp = ch == '?' ? "backward" : "forward";
				free(search);
				search = uiprompt(statusbar.x, statusbar.y,
				                  "Search (%s):", tmp);
				statusbar.dirty = 1;
			}
			if (!search || !p->nrows)
				break;

			if (ch == '/' || ch == 'n') {
				/* forward */
				for (pos = p->pos + 1; pos < p->nrows; pos++) {
					if (pane_row_match(p, pane_row_get(p, pos), search)) {
						pane_setpos(p, pos);
						break;
					}
				}
			} else {
				/* backward */
				for (pos = p->pos - 1; pos >= 0; pos--) {
					if (pane_row_match(p, pane_row_get(p, pos), search)) {
						pane_setpos(p, pos);
						break;
					}
				}
			}
			break;
		case 12: /* ^L, redraw */
			alldirty();
			break;
		case 'R': /* reload all files */
			feeds_reloadall();
			break;
		case 'a': /* attachment */
		case 'e': /* enclosure */
		case '@':
			if (selpane == PaneItems)
				feed_plumb_selected_item(&panes[selpane], FieldEnclosure);
			break;
		case 'm': /* toggle mouse mode */
			usemouse = !usemouse;
			mousemode(usemouse);
			break;
		case '<': /* decrease fixed sidebar width */
		case '>': /* increase fixed sidebar width */
			adjustsidebarsize(ch == '<' ? -1 : +1);
			break;
		case '=': /* reset fixed sidebar to automatic size */
			fixedsidebarsizes[layout] = -1;
			updategeom();
			break;
		case 't': /* toggle showing only new in sidebar */
			p = &panes[PaneFeeds];
			if ((row = pane_row_get(p, p->pos)))
				f = row->data;
			else
				f = NULL;

			onlynew = !onlynew;
			updatesidebar();

			/* try to find the same feed in the pane */
			if (f && f->totalnew &&
			    (pos = feeds_row_get(p, f)) != -1)
				pane_setpos(p, pos);
			else
				pane_setpos(p, 0);
			break;
		case 'o': /* feeds: load, items: plumb URL */
		case '\n':
openitem:
			if (selpane == PaneFeeds && panes[selpane].nrows)
				feed_open_selected(&panes[selpane]);
			else if (selpane == PaneItems && panes[selpane].nrows)
				feed_plumb_selected_item(&panes[selpane], FieldLink);
			break;
		case 'c': /* items: pipe TSV line to program */
		case 'p':
		case '|':
			if (selpane == PaneItems)
				feed_pipe_selected_item(&panes[selpane]);
			break;
		case 'y': /* yank: pipe TSV field to yank URL to clipboard */
		case 'E': /* yank: pipe TSV field to yank enclosure to clipboard */
			if (selpane == PaneItems)
				feed_yank_selected_item(&panes[selpane],
				                        ch == 'y' ? FieldLink : FieldEnclosure);
			break;
		case 'f': /* mark all read */
		case 'F': /* mark all unread */
			if (panes[PaneItems].nrows) {
				p = &panes[PaneItems];
				markread(p, 0, p->nrows - 1, ch == 'f');
			}
			break;
		case 'r': /* mark item as read */
		case 'u': /* mark item as unread */
			if (selpane == PaneItems && panes[selpane].nrows) {
				p = &panes[selpane];
				markread(p, p->pos, p->pos, ch == 'r');
			}
			break;
		case 's': /* toggle layout between monocle or non-monocle */
			setlayout(layout == LayoutMonocle ? prevlayout : LayoutMonocle);
			updategeom();
			break;
		case '1': /* vertical layout */
		case '2': /* horizontal layout */
		case '3': /* monocle layout */
			setlayout(ch - '1');
			updategeom();
			break;
		case 4: /* EOT */
		case 'q': goto end;
		}
event:
		if (ch == EOF)
			goto end;
		else if (ch == -3 && sigstate == 0)
			continue; /* just a time-out, nothing to do */

		switch (sigstate) {
		case SIGHUP:
			feeds_reloadall();
			sigstate = 0;
			break;
		case SIGINT:
		case SIGTERM:
			cleanup();
			_exit(128 + sigstate);
		case SIGWINCH:
			resizewin();
			updategeom();
			sigstate = 0;
			break;
		}

		draw();
	}
end:
	cleanup();

	return 0;
}
