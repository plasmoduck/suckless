#include <errno.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>
#include <scc/arg.h>
#include "as.h"

char *argv0;
char *outfile, *infile;
int endpass;

static void
writeout(char *fname)
{
	Section *sp;
	FILE *fp;

	if ((fp = fopen(fname, "wb")) == NULL)
		goto error;

	for (sp = seclist; sp; sp = sp->next) {
		if (!sp->mem)
			continue;
		fwrite(sp->mem, sp->max - sp->base, 1, fp);
	}

	if (fclose(fp))
		goto error;
	return;

error:
	fprintf(stderr, "as: %s: %s\n", fname, strerror(errno));
	exit(EXIT_FAILURE);
}

static void
cleanup(void)
{
	if (outfile)
		remove(outfile);
}

static int
cmp(const void *f1, const void *f2)
{
	const Ins *ins = f2;
	const char *s = f1;
	int d;

	if ((d = *s - *ins->str) != 0)
		return d;

	return strcmp(s, ins->str);
}

static void
as(char *text, char *xargs)
{
	int c;
	char *p;
	Ins *ins;
	Op *op, *lim;
	Node **args;

	for (p = text; c = *p; ++p)
		*p = toupper(c);

	ins = bsearch(text, instab, nr_ins, sizeof(Ins), cmp);
	if (!ins) {
		error("invalid instruction '%s'", text);
		return;
	}

	args = getargs(xargs);
	lim = &optab[ins->end];
	for (op = &optab[ins->begin]; op < lim; ++op) {
		if (match(op, args))
			break;
	}
	if (op == lim) {
		error("invalid operands for '%s'", text);
		return;
	}
	(*op->format)(op, args);
}

static int
dopass(char *fname)
{
	struct line line;
	extern int nerrors;
	extern jmp_buf recover;

	addinput(fname);
	cleansecs();

	endpass = 0;
	setjmp(recover);
	while (!endpass && nextline(&line)) {
		linesym = NULL;

		if (line.label)
			linesym = deflabel(line.label);

		if (line.op)
			as(line.op, line.args);
		else if (line.args)
			error("arguments without an opcode");
	}

	return nerrors == 0;
}

static void
usage(void)
{
	fputs("usage: as [-o outfile] filename ...\n", stderr);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char **p;

	outfile = "a.out";

	ARGBEGIN {
	case 'o':
		outfile = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	if (argc == 0)
		usage();

	atexit(cleanup);
	iarch();
	isecs();

	for (pass = 1; pass <= 2; pass++) {
		for (p = argv; infile = *p; ++p) {
			if (!dopass(infile))
				return EXIT_FAILURE;
		}
		if (pass == 1)
			killtmp();
	}
	writeout(outfile);
	outfile = NULL;

	return 0;
}
