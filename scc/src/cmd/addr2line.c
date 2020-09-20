#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/arg.h>
#include <scc/mach.h>

static int status;
static char *filename = "a.out";
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
addr2line(Obj *obj, char *s)
{
	int line;
	unsigned long long addr;
	char *end;
	char fname[FILENAME_MAX];

	addr = strtoull(s, &end, 16);
	if (*end  || addr == ULONG_MAX) {
		error("invalid address: '%s'", s);
		return;
	}

	if (pc2line(obj, addr, fname, &line) < 0) {
		error("not matching line");
		return;
	}

	printf("%s:%d\n", fname, line);
}

static char *
getline(void)
{
	size_t len;
	static char buf[BUFSIZ];

	for (;;) {
		if (!fgets(buf, sizeof(buf), stdin)) {
			error(strerror(errno));
			return NULL;
		}
		if ((len = strlen(buf)) == 0)
			continue;
		if (buf[len-1] != '\n') {
			error("too long address");
			continue;
		}
		buf[len-1] = '\0';

		return buf;
	}
}

static Obj *
loadexe(char *fname)
{
	int t;
	FILE *fp;
	Obj *obj;

	if ((fp = fopen(fname, "rb")) == NULL) {
		error(strerror(errno));
		return NULL;
	}

	if ((t = objtype(fp, NULL)) < 0) {
		error("file format not recognized");
		return NULL;
	}

	if ((obj = newobj(t)) == NULL) {
		error("out of memory");
		return NULL;
	}

	if (readobj(obj, fp) < 0) {
		error("file corrupted");
		return NULL;
	}

	return obj;
}

static void
usage(void)
{
	fputs("usage: addr2line [-e file] [addr ...]\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	char *ln;
	Obj *obj;

	ARGBEGIN {
	case 'e':
		filename = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	obj = loadexe(filename);
	if (!obj)
		return status;

	if (argc > 0) {
		for ( ; *argv; ++argv)
			addr2line(obj, *argv);
	} else {
		while ((ln = getline()) != NULL)
			addr2line(obj, ln);
	}

	fflush(stdout);
	if (ferror(stdout)) {
		filename = "stdout";
		error("error writing stdout: %s", strerror(errno));
	}

	return status;
}
