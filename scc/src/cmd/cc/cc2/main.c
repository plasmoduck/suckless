#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/arg.h>
#include <scc/scc.h>
#include "cc2.h"
#include "error.h"

char *argv0;

void
error(unsigned nerror, ...)
{
	va_list va;
	va_start(va, nerror);
	vfprintf(stderr, errlist[nerror], va);
	va_end(va);
	putc('\n', stderr);
	exit(1);
}

static int
moreinput(void)
{
	int c;

repeat:
	if (feof(stdin))
		return 0;
	if ((c = getchar()) == '\n' || c == EOF)
		goto repeat;
	ungetc(c, stdin);
	return 1;
}

static void
usage(void)
{
	fputs("usage: cc2 [irfile]\n", stderr);
	exit(1);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argv[0] && !freopen(argv[0], "r", stdin))
		die("cc2: %s: %s", argv[0], strerror(errno));

	while (moreinput()) {
		parse();
		apply(optm_ind);
		apply(optm_dep);
		apply(sethi);
		apply(cgen);
		getbblocks();  /* TODO: run apply over asm ins too */
		peephole();
		writeout();
	}
	return 0;
}
