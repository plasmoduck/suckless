#define _POSIX_C_SOURCE 199309L
//#define DEBUG

#include <Imlib2.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  Window root;
  Pixmap pixmap;
  Imlib_Context *render_context;
  int width, height;
} Monitor;

void setRootAtoms(Display *display, Monitor *monitor) {
  Atom atom_root, atom_eroot, type;
  unsigned char *data_root, *data_eroot;
  int format;
  unsigned long length, after;

  atom_root = XInternAtom(display, "_XROOTMAP_ID", True);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);

  // doing this to clean up after old background
  if (atom_root != None && atom_eroot != None) {
    XGetWindowProperty(display, monitor->root, atom_root, 0L, 1L, False,
                       AnyPropertyType, &type, &format, &length, &after,
                       &data_root);

    if (type == XA_PIXMAP) {
      XGetWindowProperty(display, monitor->root, atom_eroot, 0L, 1L, False,
                         AnyPropertyType, &type, &format, &length, &after,
                         &data_eroot);

      if (data_root && data_eroot && type == XA_PIXMAP &&
          *((Pixmap *)data_root) == *((Pixmap *)data_eroot))
        XKillClient(display, *((Pixmap *)data_root));
    }
  }

  atom_root = XInternAtom(display, "_XROOTPMAP_ID", False);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);

  // setting new background atoms
  XChangeProperty(display, monitor->root, atom_root, XA_PIXMAP, 32,
                  PropModeReplace, (unsigned char *)&monitor->pixmap, 1);
  XChangeProperty(display, monitor->root, atom_eroot, XA_PIXMAP, 32,
                  PropModeReplace, (unsigned char *)&monitor->pixmap, 1);
}

int main(int argc, char *argv[]) {

#ifdef DEBUG
  fprintf(stdout, "Loading images");
#endif
  Imlib_Image images[] = {
      imlib_load_image("/home/gif/raining/raining.gif"),
  };
  int images_count = 1;

#ifdef DEBUG
  fprintf(stdout, "Loading monitors\n");
#endif

  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Could not  open XDisplay\n");
    exit(42);
  }

  const int screen_count = ScreenCount(display);
#ifdef DEBUG
  fprintf(stdout, "Found %d screens\n", screen_count);
#endif

  Monitor *monitors = malloc(sizeof(Monitor) * screen_count);
  for (int current_screen = 0; current_screen < screen_count;
       ++current_screen) {
#ifdef DEBUG
    fprintf(stdout, "Running screen %d\n", current_screen);
#endif

    const int width = DisplayWidth(display, current_screen);
    const int height = DisplayHeight(display, current_screen);
    const int depth = DefaultDepth(display, current_screen);
    Visual *vis = DefaultVisual(display, current_screen);
    const int cm = DefaultColormap(display, current_screen);

#ifdef DEBUG
    fprintf(stdout, "Screen %d: width: %d, height: %d, depth: %d\n",
            current_screen, width, height, depth);
#endif

    Window root = RootWindow(display, current_screen);
    Pixmap pixmap = XCreatePixmap(display, root, width, height, depth);

    monitors[current_screen].width = width;
    monitors[current_screen].height = height;
    monitors[current_screen].root = root;
    monitors[current_screen].pixmap = pixmap;
    monitors[current_screen].render_context = imlib_context_new();
    imlib_context_push(monitors[current_screen].render_context);
    imlib_context_set_display(display);
    imlib_context_set_visual(vis);
    imlib_context_set_colormap(cm);
    imlib_context_set_drawable(pixmap);
    imlib_context_set_color_range(imlib_create_color_range());
    imlib_context_pop();
  }

#ifdef DEBUG
  fprintf(stdout, "Loaded %d screens\n", screen_count);
#endif

#ifdef DEBUG
  fprintf(stdout, "Starting render loop");
#endif

  struct timespec timeout;
  timeout.tv_sec = 0;
  timeout.tv_nsec = 33000000;

  for (int cycle = 0; cycle < 10; ++cycle) {
    Imlib_Image current = images[cycle % images_count];
    for (int monitor = 0; monitor < screen_count; ++monitor) {
      Monitor *c_monitor = &monitors[monitor];
      imlib_context_push(c_monitor->render_context);
      imlib_context_set_dither(1);
      imlib_context_set_blend(1);
      imlib_context_set_image(current);

      imlib_render_image_on_drawable(0, 0);

      setRootAtoms(display, c_monitor);
      XKillClient(display, AllTemporary);
      XSetCloseDownMode(display, RetainTemporary);
      XSetWindowBackgroundPixmap(display, c_monitor->root, c_monitor->pixmap);
      XClearWindow(display, c_monitor->root);
      XFlush(display);
      XSync(display, False);
      imlib_context_pop();
    }
    nanosleep(&timeout, NULL);
  }
}
