#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static char *getprop(Atom atom);
static void setprop(Atom atom, char *value);

static Atom utf8;
static Display *dpy;
static Window win;

int
main(int argc, char *argv[])
{
	char *value = NULL;
	Atom atom;

	if(!(dpy = XOpenDisplay(NULL))) {
		fputs("sprop: cannot open display\n", stderr);
		return 1;
	}
	switch(argc) {
	case 4:
		value = argv[3];
	case 3:
		atom = XInternAtom(dpy, argv[2], True);
		utf8 = XInternAtom(dpy, "UTF8_STRING", True);
		win = atol(argv[1]);
		break;
	case 2:
		if(!strcmp(argv[1], "-v")) {
			fputs("sprop-"VERSION", © 2010 Connor Lane Smith\n", stdout);
			return 0;
		}
	default:
		fprintf(stderr, "usage: sprop <xid> <atom> [<value>] [-v]\n");
		return 1;
	}
	if(value)
		setprop(atom, value);
	else {
		if(!(value = getprop(atom))) {
			fputs("sprop: cannot get atom\n", stderr);
			return 1;
		}
		printf("%s\n", value);
		XFree(value);
	}
	XCloseDisplay(dpy);
	return 0;
}

char *
getprop(Atom atom)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da;

	XGetWindowProperty(dpy, win, atom, 0, BUFSIZ, False, utf8, &da, &di, &dl, &dl, &p);
	return (char *)p;
}

void
setprop(Atom atom, char *value)
{
	XChangeProperty(dpy, win, atom, utf8, 8, PropModeReplace,
			(unsigned char *)value, strlen(value));
}
