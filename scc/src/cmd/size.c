#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/ar.h>
#include <scc/arg.h>
#include <scc/mach.h>

struct sizes {
	unsigned long long text;
	unsigned long long data;
	unsigned long long bss;
};

static int status;
static char *filename, *membname;
static int tflag;
static unsigned long long ttext, tdata, tbss, ttotal;
char *argv0;

static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "size: %s: ", filename);
	if (membname)
		fprintf(stderr, "%s: ", membname);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	status = EXIT_FAILURE;
}

static void
sizeobj(FILE *fp, int type)
{
	int i;
	Obj *obj;
	unsigned long long total, *p;
	struct sizes siz;
	Section sec;

	if ((obj = newobj(type)) == NULL) {
		error(strerror(errno));
		return;
	}

	if (readobj(obj, fp) < 0) {
		error(strerror(errno));
		goto err;
	}

	siz.text = siz.data = siz.bss = 0;
	for (i = 0; getsec(obj, &i, &sec); i++) {
		switch (sec.type) {
		case 'R':
		case 'T':
			p = &siz.text;
			break;
		case 'D':
			p = &siz.data;
			break;
		case 'B':
			p = &siz.bss;
			break;
		default:
			continue;
		}

		if (*p > ULLONG_MAX - sec.size) {
			error("integer overflow");
			goto err;
		}
			
		*p += sec.size;
	}

	total = siz.text + siz.data + siz.bss;
	printf("%llu\t%llu\t%llu\t%llu\t%llx\t%s\n",
	       siz.text,
	       siz.data,
	       siz.bss,
	       total, total, filename);

	ttext += siz.text;
	tdata += siz.data;
	tbss += siz.bss;
	ttotal += total;

err:
	delobj(obj);
}

static void
sizelib(FILE *fp)
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
				sizeobj(fp, t);
			membname = NULL;
			fseek(fp, cur, SEEK_SET);
			fseek(fp, off, SEEK_CUR);
			break;
		}
	}
}

static void
size(char *fname)
{
	int t;
	FILE *fp;

	filename = fname;
	if ((fp = fopen(fname, "rb")) == NULL) {
		error(strerror(errno));
		return;
	}

	if ((t = objtype(fp, NULL)) != -1)
		sizeobj(fp, t);
	else if (archive(fp))
		sizelib(fp);
	else
		error("bad format");

	fclose(fp);
}

static void
usage(void)
{
	fputs("usage: size [-t] [file...]\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	unsigned long long total;

	ARGBEGIN {
	case 't':
		tflag = 1;
		break;
	default:
		usage();
	} ARGEND

	puts("text\tdata\tbss\tdec\thex\tfilename");

	if (argc == 0) {
		size("a.out");
	} else {
		for (; *argv; ++argv)
			size(*argv);
	}

	if (tflag) {
		total = ttext + tdata + tbss;
		printf("%llu\t%llu\t%llu\t%llu\t%llx\t%s\n",
		       ttext, tdata, tbss, total, total, "(TOTALS)");
	}

	if (fflush(stdout)) {
		fprintf(stderr,
		        "size: error writing in output:%s\n",
		        strerror(errno));
		status = 1;
	}

	return status;
}
