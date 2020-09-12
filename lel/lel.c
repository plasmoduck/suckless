/* See LICENSE file for copyright and license details. */
#include <arpa/inet.h>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "arg.h"
char *argv0;

#define APP_NAME "lel"
#define HEADER_FORMAT "farbfeld########"

/* Image status flags. */
enum { NONE = 0, LOADED = 1, SCALED = 2, DRAWN = 4 };
/* View mode. */
enum { ASPECT = 0, FULL_ASPECT, FULL_STRETCH };

struct img {
	char *filename;
	FILE *fp;
	int state;
	int width;
	int height;
	uint8_t *buf;
	struct view {
		int panxoffset;
		int panyoffset;
		float zoomfact;
	} view;
};

static struct img *imgs;
static struct img *cimg;
static size_t nimgs;
static int viewmode = ASPECT;
static char *wintitle = APP_NAME;
static char *bgcolor = "#000000";
static XImage *ximg = NULL;
static Drawable xpix = 0;
static Display *dpy = NULL;
static Colormap cmap;
static Window win;
static GC gc;
static XColor bg;
static int screen, xfd;
static int running = 1;
static int winwidth = 0, winheight = 0;
static int winx, winy, reqwinwidth = 320, reqwinheight = 240;
static float zoominc = 0.25;
static int tflag;
static int wflag;
static int hflag;

static void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	}
	exit(1);
}

static void
usage(void)
{
	die("%s", APP_NAME " " VERSION "\n\n"
	      "usage: " APP_NAME " [OPTIONS...] [FILE]\n"
	      "    -a            Full window, keep aspect ratio\n"
	      "    -f            Full window, stretch (no aspect)\n"
	      "    -w <w>        Window width\n"
	      "    -h <h>        Window height\n"
	      "    -x <x>        Window x position\n"
	      "    -y <y>        Window y position\n"
	      "    -t <title>    Use title\n"
	      "    -v            Print version and exit\n");
}

static int
ff_open(struct img *img)
{
	uint8_t hdr[17];

	if (img->state & LOADED)
		return 0;

	if (fread(hdr, 1, strlen(HEADER_FORMAT), img->fp) != strlen(HEADER_FORMAT))
		return -1;

	if (memcmp(hdr, "farbfeld", 8))
		return -1;

	img->width = ntohl((hdr[8] << 0) | (hdr[9] << 8) | (hdr[10] << 16) | (hdr[11] << 24));
	img->height = ntohl((hdr[12] << 0) | (hdr[13] << 8) | (hdr[14] << 16) | (hdr[15] << 24));
	if (img->width <= 0 || img->height <= 0)
		return -1;

	if (!(img->buf = malloc(img->width * img->height * 4)))
		die("malloc:");

	return 0;
}

static int
ff_read(struct img *img)
{
	int i, j, off, row_len;
	uint16_t *row;

	if (img->state & LOADED)
		return 0;

	row_len = img->width * strlen("RRGGBBAA");
	if (!(row = malloc(row_len)))
		return -1;

	for (off = 0, i = 0; i < img->height; ++i) {
		if (fread(row, 1, (size_t)row_len, img->fp) != (size_t)row_len) {
			free(row);
			die("unexpected EOF or row-skew at %d\n", i);
		}
		for (j = 0; j < row_len / 2; j += 4, off += 4) {
			img->buf[off]     = row[j];
			img->buf[off + 1] = row[j + 1];
			img->buf[off + 2] = row[j + 2];
			img->buf[off + 3] = row[j + 3];
		}
	}
	free(row);

	img->state |= LOADED;

	return 0;
}

static void
ff_close(struct img *img)
{
	img->state &= ~LOADED;
	rewind(img->fp);
	free(img->buf);
}

/* NOTE: will be removed later, for debugging alpha mask */
#if 0
static void
normalsize(char *newbuf)
{
	unsigned int x, y, soff = 0, doff = 0;

	for (y = 0; y < cimg->height; y++) {
		for (x = 0; x < cimg->width; x++, soff += 4, doff += 4) {
			newbuf[doff+0] = cimg->buf[soff+2];
			newbuf[doff+1] = cimg->buf[soff+1];
			newbuf[doff+2] = cimg->buf[soff+0];
			newbuf[doff+3] = cimg->buf[soff+3];
		}
	}
}
#endif

static void
loadimg(void)
{
	if (ff_open(cimg))
		die("can't open image (invalid format?)\n");
	if (ff_read(cimg))
		die("can't read image\n");
	if (!wflag)
		reqwinwidth = cimg->width;
	if (!hflag)
		reqwinheight = cimg->height;
	if (!tflag)
		wintitle = cimg->filename;
}

static void
reloadimg(void)
{
	loadimg();
	XResizeWindow(dpy, win, reqwinwidth, reqwinheight);
	XStoreName(dpy, win, wintitle);
	XFlush(dpy);
}

static void
nextimg(void)
{
	struct img *tmp = cimg;

	cimg++;
	if (cimg >= &imgs[nimgs])
		cimg = &imgs[0];
	if (tmp != cimg) {
		ff_close(tmp);
		reloadimg();
	}
}

static void
previmg(void)
{
	struct img *tmp = cimg;

	cimg--;
	if (cimg < &imgs[0])
		cimg = &imgs[nimgs - 1];
	if (tmp != cimg) {
		ff_close(tmp);
		reloadimg();
	}
}

/* scales imgbuf data to newbuf (ximg->data), nearest neighbour. */
static void
scale(unsigned int width, unsigned int height, unsigned int bytesperline,
	char *newbuf)
{
	unsigned char *ibuf;
	unsigned int jdy, dx, bufx, x, y;
	float a = 0.0f;

	jdy = bytesperline / 4 - width;
	dx = (cimg->width << 10) / width;
	for (y = 0; y < height; y++) {
		bufx = cimg->width / width;
		ibuf = &cimg->buf[y * cimg->height / height * cimg->width * 4];

		for (x = 0; x < width; x++) {
			a = (ibuf[(bufx >> 10)*4+3]) / 255.0f;
			*newbuf++ = (ibuf[(bufx >> 10)*4+2] * a) + (bg.blue * (1 - a));
			*newbuf++ = (ibuf[(bufx >> 10)*4+1] * a) + (bg.green * (1 - a));
			*newbuf++ = (ibuf[(bufx >> 10)*4+0] * a) + (bg.red * (1 - a));
			newbuf++;
			bufx += dx;
		}
		newbuf += jdy;
	}
}

static void
ximage(unsigned int newwidth, unsigned int newheight)
{
	int depth;

	/* destroy previous image */
	if (ximg) {
		XDestroyImage(ximg);
		ximg = NULL;
	}
	depth = DefaultDepth(dpy, screen);
	if (depth >= 24) {
		if (xpix)
			XFreePixmap(dpy, xpix);
		xpix = XCreatePixmap(dpy, win, winwidth, winheight, depth);
		ximg = XCreateImage(dpy, CopyFromParent, depth,	ZPixmap, 0,
		                    NULL, newwidth, newheight, 32, 0);
		ximg->data = malloc(ximg->bytes_per_line * ximg->height);
		scale(ximg->width, ximg->height, ximg->bytes_per_line, ximg->data);
		XInitImage(ximg);
	} else {
		die("this program does not yet support display depths < 24\n");
	}
}

static void
scaleview(void)
{
	switch(viewmode) {
	case FULL_STRETCH:
		ximage(winwidth, winheight);
		break;
	case FULL_ASPECT:
		if (winwidth * cimg->height > winheight * cimg->width)
			ximage(cimg->width * winheight / cimg->height, winheight);
		else
			ximage(winwidth, cimg->height * winwidth / cimg->width);
		break;
	case ASPECT:
	default:
		ximage(cimg->width * cimg->view.zoomfact, cimg->height * cimg->view.zoomfact);
		break;
	}
	cimg->state |= SCALED;
}

static void
draw(void)
{
	int xoffset = 0, yoffset = 0;

	if (viewmode != FULL_STRETCH) {
		/* center vertical, horizontal */
		xoffset = (winwidth - ximg->width) / 2;
		yoffset = (winheight - ximg->height) / 2;
		/* pan offset */
		xoffset -= cimg->view.panxoffset;
		yoffset -= cimg->view.panyoffset;
	}
	XSetForeground(dpy, gc, bg.pixel);
	XFillRectangle(dpy, xpix, gc, 0, 0, winwidth, winheight);
	XPutImage(dpy, xpix, gc, ximg, 0, 0, xoffset, yoffset, ximg->width, ximg->height);
	XCopyArea(dpy, xpix, win, gc, 0, 0, winwidth, winheight, 0, 0);

	XFlush(dpy);
	cimg->state |= DRAWN;
}

static void
update(void)
{
	if (!(cimg->state & LOADED))
		return;
	if (!(cimg->state & SCALED))
		scaleview();
	if (!(cimg->state & DRAWN))
		draw();
}

static void
setview(int mode)
{
	if (viewmode == mode)
		return;
	viewmode = mode;
	cimg->state &= ~(DRAWN | SCALED);
	update();
}

static void
pan(int x, int y)
{
	cimg->view.panxoffset -= x;
	cimg->view.panyoffset -= y;
	cimg->state &= ~(DRAWN | SCALED);
	update();
}

static void
inczoom(float f)
{
	if ((cimg->view.zoomfact + f) <= 0)
		return;
	cimg->view.zoomfact += f;
	cimg->state &= ~(DRAWN | SCALED);
	update();
}

static void
zoom(float f)
{
	if (f == cimg->view.zoomfact)
		return;
	cimg->view.zoomfact = f;
	cimg->state &= ~(DRAWN | SCALED);
	update();
}

static void
buttonpress(XEvent *ev)
{
	switch(ev->xbutton.button) {
	case Button4:
		inczoom(zoominc);
		break;
	case Button5:
		inczoom(-zoominc);
		break;
	}
}

static void
printname(void)
{
	printf("%s\n", cimg->filename);
}

static void
keypress(XEvent *ev)
{
	KeySym key;

	key = XLookupKeysym(&ev->xkey, 0);
	switch(key) {
	case XK_Escape:
	case XK_q:
		running = 0;
		break;
	case XK_Left:
	case XK_h:
		pan(winwidth / 20, 0);
		break;
	case XK_Down:
	case XK_j:
		pan(0, -(winheight / 20));
		break;
	case XK_Up:
	case XK_k:
		pan(0, winheight / 20);
		break;
	case XK_Right:
	case XK_l:
		pan(-(winwidth / 20), 0);
		break;
	case XK_a:
		setview(FULL_ASPECT);
		break;
	case XK_o:
		setview(ASPECT);
		break;
	case XK_Return:
		printname();
		break;
	case XK_f:
		setview(FULL_STRETCH);
		break;
	case XK_KP_Add:
	case XK_equal:
	case XK_plus:
		inczoom(zoominc);
		break;
	case XK_KP_Subtract:
	case XK_underscore:
	case XK_minus:
		inczoom(-zoominc);
		break;
	case XK_3:
		zoom(4.0);
		break;
	case XK_2:
		zoom(2.0);
		break;
	case XK_1:
		zoom(1.0);
		break;
	case XK_0:
		zoom(1.0);
		setview(ASPECT); /* fallthrough */
	case XK_r:
		cimg->view.panxoffset = 0;
		cimg->view.panyoffset = 0;
		cimg->state &= ~(DRAWN | SCALED);
		update();
		break;
	case XK_n:
		nextimg();
		cimg->state &= ~(DRAWN | SCALED);
		update();
		break;
	case XK_p:
		previmg();
		cimg->state &= ~(DRAWN | SCALED);
		update();
		break;
	}
}

static void
handleevent(XEvent *ev)
{
	XWindowAttributes attr;

	switch(ev->type) {
	case MapNotify:
		if (!winwidth || !winheight) {
			XGetWindowAttributes(ev->xmap.display, ev->xmap.window, &attr);
			winwidth = attr.width;
			winheight = attr.height;
		}
		break;
	case ConfigureNotify:
		if (winwidth != ev->xconfigure.width || winheight != ev->xconfigure.height) {
			winwidth = ev->xconfigure.width;
			winheight = ev->xconfigure.height;
			cimg->state &= ~(SCALED);
		}
		break;
	case Expose:
		cimg->state &= ~(DRAWN);
		update();
		break;
	case KeyPress:
		keypress(ev);
		break;
	case ButtonPress:
		buttonpress(ev);
		break;
	}
}

static void
setup(void)
{
	XClassHint class = { APP_NAME, APP_NAME };

	if (!(dpy = XOpenDisplay(NULL)))
		die("can't open X display\n");
	xfd = ConnectionNumber(dpy);
	screen = DefaultScreen(dpy);

	win = XCreateWindow(dpy, DefaultRootWindow(dpy), winx, winy, reqwinwidth, reqwinheight, 0,
	                    DefaultDepth(dpy, screen), InputOutput,
	                    CopyFromParent, 0, NULL);
	gc = XCreateGC(dpy, win, 0, NULL);
	cmap = DefaultColormap(dpy, screen);
	if (!XAllocNamedColor(dpy, cmap, bgcolor, &bg, &bg))
		die("cannot allocate color\n");
	XStoreName(dpy, win, wintitle);
	XSelectInput(dpy, win, StructureNotifyMask | ExposureMask | KeyPressMask |
	                       ButtonPressMask);
	XMapRaised(dpy, win);
	XSetWMProperties(dpy, win, NULL, NULL, NULL, 0, NULL, NULL, &class);
	XFlush(dpy);
}

static void
run(void)
{
	XEvent ev;

	while (running && !XNextEvent(dpy, &ev))
		handleevent(&ev);
}

int
main(int argc, char *argv[]) {
	FILE *fp;
	int i, j;

	ARGBEGIN {
	case 'a':
		viewmode = FULL_ASPECT;
		break;
	case 'f':
		viewmode = FULL_STRETCH;
		break;
	case 'h':
		hflag = 1;
		if (!(reqwinheight = atoi(EARGF(usage()))))
			usage();
		break;
	case 't':
		wintitle = EARGF(usage());
		tflag = 1;
		break;
	case 'w':
		wflag = 1;
		if (!(reqwinwidth = atoi(EARGF(usage()))))
			usage();
		break;
	case 'x':
		winx = atoi(EARGF(usage()));
		break;
	case 'y':
		winy = atoi(EARGF(usage()));
		break;
	default:
		usage();
		break;
	} ARGEND;

	if (!argc) {
		imgs = calloc(1, sizeof(*imgs));
		if (!imgs)
			die("calloc:");
		nimgs = 1;
		imgs[0].filename = "<stdin>";
		imgs[0].fp = stdin;
		imgs[0].view.zoomfact = 1.0;
	} else {
		imgs = calloc(argc, sizeof(*imgs));
		if (!imgs)
			die("calloc:");
		for (i = 0, j = 0; j < argc; j++) {
			fp = fopen(argv[j], "rb");
			if (!fp) {
				fprintf(stderr, "can't open %s: %s\n", argv[j],
					strerror(errno));
				continue;
			}
			imgs[i].filename = argv[j];
			imgs[i].fp = fp;
			imgs[i].view.zoomfact = 1.0;
			i++;
		}
		if (!i)
			return 1;
		nimgs = i;
	}
	cimg = imgs;

	loadimg();
	setup();
	run();

	return 0;
}
