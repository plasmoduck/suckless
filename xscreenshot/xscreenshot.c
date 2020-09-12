#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int
main(int argc, char *argv[])
{
	XImage *img;
	Display *dpy;
	Window win;
	XWindowAttributes attr;
	uint32_t tmp, w, h;
	uint16_t rgba[4];
	int sr, sg, fr, fg, fb;
	char *ep;

	if (!(dpy = XOpenDisplay(NULL)))
		errx(1, "XOpenDisplay");

	/* identify window */
	if (argc > 1) {
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-v")) {
			fprintf(stderr, "usage: %s [winid]\n", argv[0]);
			return 1;
		}
		errno = 0;

		win = (Window)strtol(argv[1], &ep, 0);
		if (errno || argv[1] == ep || *ep != '\0') {
			fprintf(stderr, "strtol: invalid number: \"%s\"%s%s\n",
				argv[1],
			        errno ? ": " : "",
			        errno ? strerror(errno) : "");
			exit(1);
		}
	} else {
		win = RootWindow(dpy, 0);
	}

	XGrabServer(dpy);
	XGetWindowAttributes(dpy, win, &attr);
	img = XGetImage(dpy, win, 0, 0, attr.width, attr.height, 0xffffffff,
	                ZPixmap);
	XUngrabServer(dpy);
	XCloseDisplay(dpy);
	if (!img)
		errx(1, "XGetImage");

	switch (img->bits_per_pixel) {
	case 16: /* only 5-6-5 format supported */
		sr = 11;
		sg = 5;
		fr = fb = 2047;
		fg = 1023;
		break;
	case 24:
	case 32: /* ignore alpha in case of 32-bit */
		sr = 16;
		sg = 8;
		fr = fg = fb = 257;
		break;
	default:
		errx(1, "unsupported bpp: %d", img->bits_per_pixel);
	}

	/* write header with big endian width and height-values */
	fprintf(stdout, "farbfeld");
	tmp = htonl(img->width);
	fwrite(&tmp, sizeof(uint32_t), 1, stdout);
	tmp = htonl(img->height);
	fwrite(&tmp, sizeof(uint32_t), 1, stdout);

	/* write pixels */
	for (h = 0; h < (uint32_t)img->height; h++) {
		for (w = 0; w < (uint32_t)img->width; w++) {
			tmp = XGetPixel(img, w, h);
			rgba[0] = htons(((tmp & img->red_mask) >> sr) * fr);
			rgba[1] = htons(((tmp & img->green_mask) >> sg) * fg);
			rgba[2] = htons((tmp & img->blue_mask) * fb);
			rgba[3] = htons(65535);

			if (fwrite(&rgba, 4 * sizeof(uint16_t), 1, stdout) != 1)
				err(1, "fwrite");
		}
	}
	XDestroyImage(img);

	return 0;
}
