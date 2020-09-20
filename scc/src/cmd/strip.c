#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/arg.h>
#include <scc/mach.h>

static int status;
static char tmpname[FILENAME_MAX];
static char *filename;
char *argv0;

static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "strip: %s: ", filename);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	status = EXIT_FAILURE;
}

static void
doit(char *fname)
{
	int type;;
	size_t r;
	FILE *src, *dst;
	Map *map;
	Obj *obj;

	errno = 0;
	filename = fname;
	r = snprintf(tmpname, sizeof(tmpname), "%s.tmp", fname);
	if (r >= sizeof(tmpname)) {
		error("%s: name too long", fname);
		return;
	}
	if ((src = fopen(fname, "rb")) == NULL) {
		error("opening src file");
		goto err0;
	}
	if ((type = objtype(src, NULL)) < 0) {
		error("getting object type");
		goto err1;
	}
	if ((obj = newobj(type)) == NULL) {
		error("creating object");
		goto err1;
	}
	if (readobj(obj, src) < 0) {
		error("reading object");
		goto err2;
	}
	if ((map = loadmap(obj, src)) == NULL) {
		error("loading map");
		goto err3;
	}
	if (strip(obj) < 0) {
		error("stripping");
		goto err3;
	}
	if ((dst = fopen(tmpname, "wb")) == NULL) {
		error("opening dst");
		goto err3;
	}
	if (writeobj(obj, map, dst) < 0) {
		error("writing object");
		goto err5;
	}
	if (fclose(dst) == EOF) {
		error("closing dst");
		goto err4;
	}
	if (remove(fname) == EOF) {
		error("removing fname");
		goto err4;
	}
	if (rename(tmpname, fname) == EOF) {
		error("rename fname");
		goto err4;
	}

	goto err3;

err5:
	fclose(dst);
err4:
	remove(tmpname);
err3:
	free(map);
err2:
	delobj(obj);
err1:
	fclose(src);
err0:
	error("cannot strip it");
}

static void
usage(void)
{
	fputs("usage: strip [file...]\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argc == 0) {
		doit("a.out");
	} else {
		for (; *argv; ++argv)
			doit(*argv);
	}

	return status;
}
