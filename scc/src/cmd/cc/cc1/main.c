#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <scc/arg.h>
#include <scc/scc.h>
#include "cc1.h"

char *argv0, *infile;

int warnings;
jmp_buf recover;

int onlycpp, onlyheader;


extern int failure;

static void
defmacro(char *macro)
{
	char *p = strchr(macro, '=');

	if (p)
		*p++ = '\0';
	else
		p = "1";

	defdefine(macro, p, "command-line");
}

static void
usage(void)
{
	fputs("usage: cc1 [-Ewd] [-D def[=val]]... [-U def]... "
	      "[-I dir]... [-a architecture] [input]\n", stderr);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int i;
	static struct items uflags, dflags, iflags;

	ARGBEGIN {
	case 'a':
		architecture = EARGF(usage());
		break;
	case 'D':
		newitem(&dflags, EARGF(usage()));
		break;
	case 'M':
		onlyheader = 1;
		break;
	case 'E':
		onlycpp = 1;
		break;
	case 'I':
		newitem(&iflags, EARGF(usage()));
		break;
	case 'U':
		newitem(&uflags, EARGF(usage()));
		break;
	case 'd':
		DBGON();
		break;
	case 'w':
		warnings = 1;
		break;
	default:
		usage();
	} ARGEND

	if (argc > 1)
		usage();

	icode();
	iarch();
	ilex();
	icpp();
	ibuilts();

	for (i = 0; i < iflags.n; ++i)
		incdir(iflags.s[i]);

	for (i = 0; i < dflags.n; ++i)
		defmacro(dflags.s[i]);

	for (i = 0; i < uflags.n; ++i)
		undefmacro(uflags.s[i]);

	infile = (*argv) ? *argv : "<stdin>";
	addinput(*argv, NULL, NULL);

	if (onlycpp || onlyheader) {
		outcpp();
	} else {
		next();
		while (yytoken != EOFTOK)
			decl();
	}

	return failure;
}
