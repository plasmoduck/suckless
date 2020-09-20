#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/ar.h>
#include <scc/arg.h>
#include <scc/mach.h>


struct symtbl {
	Symbol **buf;
	size_t nsyms;
};

char *argv0;
static int status, multi;
static int radix = 16;
static int Pflag;
static int Aflag;
static int vflag;
static int gflag;
static int uflag;
static char *filename, *membname;

static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "nm: %s: ", filename);
	if (membname)
		fprintf(stderr, "%s: ", membname);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	status = EXIT_FAILURE;
}

static int
cmp(const void *p1, const void *p2)
{
	Symbol **s1 = (Symbol **) p1, **s2 = (Symbol **) p2;
	Symbol *sym1 = *s1, *sym2 = *s2;

	if (vflag) {
		if (sym1->value > sym2->value)
			return 1;
		if (sym1->value < sym2->value)
			return -1;
		if (sym1->type == 'U' && sym2->type == 'U')
			return 0;
		if (sym1->type == 'U')
			return -1;
		if (sym2->type == 'U')
			return 1;
		return 0;
	} else {
		return strcmp(sym1->name, sym2->name);
	}
}

static void
printsyms(Symbol **syms, size_t nsym)
{
	size_t i;

	if (nsym == 0) {
		error("no symbols");
		return;
	}
	qsort(syms, nsym, sizeof(syms), cmp);

	if (!Aflag) {
		if (multi || membname)
			printf("%s:\n", (membname) ? membname : filename);
	}

	for (i = 0; i < nsym; i++) {
		Symbol *sym = syms[i];
		int type = sym->type;
		char *fmt;

		if (Aflag) {
			fmt = (membname) ? "%s[%s]: " : "%s: ";
			printf(fmt, filename, membname);
		}

		if (Pflag) {
			printf("%s %c", sym->name, sym->type);
			if (type != 'U') {
				if (radix == 8)
					fmt = " %016.16llo %lo";
				else if (radix == 10)
					fmt = " %016.16llu %lu";
				else
					fmt = " %016.16llx %lx";
				printf(fmt, sym->value, sym->size);
			}
		} else {
			if (type == 'U')
				fmt = "                ";
			else if (radix == 8)
				fmt = "%016.16llo";
			else if (radix == 10)
				fmt = "%016.16lld";
			else
				fmt = "%016.16llx";
			printf(fmt, sym->value);
			printf(" %c %s", sym->type, sym->name);
		}
		putchar('\n');
	}
}

static int
newsym(Symbol *sym, struct symtbl *tbl)
{
	Symbol **p, *s;
	size_t n, size;
	int type = sym->type;

	if (type == '?'
	|| type == 'N'
	|| uflag && type != 'U'
	|| gflag && !isupper(type)) {
		return 0;
	}

	n = tbl->nsyms+1;
	size = n *sizeof(*p);
	p = realloc(tbl->buf, size);
	s = malloc(sizeof(*s));
	if (!p || !s) {
		free(p);
		free(s);
		error(strerror(errno));
		return -1;
	}

	*s = *sym;
	tbl->buf = p;
	p[tbl->nsyms++] = s;
	return 0;
}

static void
nmobj(FILE *fp, int type)
{
	int i, err = 1;
	Obj *obj;
	Symbol sym;
	struct symtbl tbl = {NULL, 0};

	if ((obj = newobj(type)) == NULL) {
		error(strerror(errno));
		goto err1;
	}

	if (readobj(obj, fp) < 0) {
		error(strerror(errno));
		goto err2;
	}

	for (i = 0; getsym(obj, &i, &sym); i++) {
		if (newsym(&sym, &tbl) < 0)
			goto err3;
	}

	printsyms(tbl.buf, tbl.nsyms);
	err = 0;

err3:
	free(tbl.buf);
err2:
	delobj(obj);
err1: 
	if (err)
		error("object file corrupted");
}

static void
nmlib(FILE *fp)
{
	int t;
	long off, cur;
	char memb[SARNAM+1];

	for (;;) {
		cur = ftell(fp);
		off = armember(fp, memb);
		switch (off) {
		case -1:
			error("library corrupted");
			if (ferror(fp))
				error(strerror(errno));
		case 0:
			return;
		default:
			membname = memb;
			if ((t = objtype(fp, NULL)) != -1)
				nmobj(fp, t);
			membname = NULL;
			fseek(fp, cur, SEEK_SET);
			fseek(fp, off, SEEK_CUR);
			break;
		}
	}
}

static void
nm(char *fname)
{
	int t;
	FILE *fp;

	filename = fname;
	membname = NULL;

	if ((fp = fopen(fname, "rb")) == NULL) {
		error(strerror(errno));
		return;
	}

	if ((t = objtype(fp, NULL)) != -1)
		nmobj(fp, t);
	else if (archive(fp))
		nmlib(fp);
	else
		error("bad format");

	fclose(fp);
}

static void
usage(void)
{
	fputs("nm [-APv][ -g| -u][-t format] [file...]\n", stderr);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *t;

	ARGBEGIN {
	case 'P':
		Pflag = 1;
		break;
	case 'A':
		Aflag = 1;
		break;
	case 'g':
		gflag = 1;
		break;
	case 'u':
		uflag = 1;
		break;
	case 'v':
		vflag = 1;
		break;
	case 't':
		t = EARGF(usage());
		if (!strcmp(t, "o"))
			radix = 8;
		else if (!strcmp(t, "d"))
			radix = 10;
		else if (!strcmp(t, "x"))
			radix = 16;
		else
			usage();
		break;
	default:
		usage();
	} ARGEND

	if (argc == 0) {
		nm("a.out");
	} else {
		if (argc > 1)
			multi = 1;
		for ( ; *argv; ++argv)
			nm(*argv);
	}

	if (fflush(stdout) == EOF) {
		fprintf(stderr,
		        "nm: error writing in output:%s\n",
		        strerror(errno));
		status = 1;
	}

	return status;
}
