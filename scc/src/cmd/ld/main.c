#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/mach.h>

#include "ld.h"

#define MAX_LIB_PATHS 12

int sflag;        /* discard all the symbols */
int xflag;        /* discard local symbols */
int Xflag;        /* discard locals starting with 'L' */
int rflag;        /* preserve relocation bits */
int dflag;        /* define common even with rflag */
int gflag;        /* preserve debug symbols */
int nmagic;       /* nmagic output */

char *filename, *membname;

Segment text = {.type = 'T'};
Segment rodata = {.type = 'R'};
Segment data = {.type = 'D'};
Segment bss = {.type = 'B'};
Segment debug = {.type = 'N'};

char *libpaths[MAX_LIB_PATHS];

char *output = "a.out", *entry = "start";
static int status;

void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "ld: %s: ", filename);
	if (membname)
		fprintf(stderr, "%s: ", membname);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	status = EXIT_FAILURE;
}

static void
cleanup(void)
{
	if (status != EXIT_FAILURE)
		remove(output);
}

/*
 * pass1: Get the list of object files that are going to be linked.
 * pass2: Calculate the size of every segment.
 * pass3: Rebase all symbols in sections
 * pass4: Create the temporary files per section
 * pass5: Create the temporary files per segment
 */
static void
ld(int argc, char*argv[])
{
	pass1(argc, argv);
	pass2(argc, argv);
/*
	pass3(argc, argv);
	pass4(argc, argv);
	pass5(argc, argv);
*/
	debugsym();
	debugsec();
}

static void
usage(void)
{
	fputs("usage: ld [options] file ...\n", stderr);
	exit(EXIT_FAILURE);
}

static void
Lpath(char *path)
{
	char **bp, **end;

	end = &libpaths[MAX_LIB_PATHS];
	for (bp = libpaths; bp < end && *bp; ++bp)
		;
	if (bp == end) {
		fputs("ld: too many -L options\n", stderr);
		exit(1);
	}
	*bp = path;
}

char *
nextarg(char **argp, char ***argv)
{
	char *ap = *argp, **av = *argv;

	if (ap[1]) {
		*argp += strlen(ap);
		return ap+1;
	}

	if (av[1]) {
		*argv = ++av;
		return *av;
	}

	usage();
}

int
main(int argc, char *argv[])
{
	int files = 0;
	char *ap, **av;

	for (av = argv+1; *av; ++av) {
		if (av[0][0] != '-') {
			files = 1;
			continue;
		}
		for (ap = &av[0][1]; *ap; ++ap) {
			switch (*ap) {
			case 's':
				sflag = 1;
				break;
			case 'x':
				xflag = 1;
				break;
			case 'X':
				Xflag = 1;
				break;
			case 'i':
			case 'r':
				rflag = 1;
				break;
			case 'd':
				dflag = 1;
				break;
			case 'n':
				nmagic = 1;
				break;
			case 'l':
			case 'u':
				nextarg(&ap, &av);
				break;
			case 'L':
				Lpath(nextarg(&ap, &av));
				break;
			case 'o':
				output = nextarg(&ap, &av);
				break;
			case 'e':
				entry = nextarg(&ap, &av);
				break;
			default:
				usage();
			}
		}
	}

	if (!files)
		usage();

	atexit(cleanup);
	ld(argc, argv);

	return status;
}
