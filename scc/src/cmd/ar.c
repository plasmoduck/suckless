#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sys.h"

#include <scc/ar.h>
#include <scc/arg.h>

enum {
	BEFORE,
	INDOT,
	AFTER,
};

struct tmp {
	char *name;
	FILE *fp;
} tmps[3];

char *argv0;

static int bflag, vflag, cflag, lflag, uflag, aflag;
static char *arfile, *posname;

struct member {
	FILE *src;
	struct ar_hdr hdr;
	int cur;
	char *fname;
	long size;
	long mode;
	long long date;
};

static void
cleanup(void)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (tmps[i].name)
			remove(tmps[i].name);
	}
}

static char *
errstr(void)
{
	return strerror(errno);
}

static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "ar: %s: ", arfile);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	exit(EXIT_FAILURE);
}

/*
 * I do know that you cannot call remove from a signal handler
 * but we only can use stdio function to deal with files
 * because we are C99 compliant, and it is very likely that
 * remove is going to work in this case
 */
static void
sigfun(int signum)
{
	cleanup();
	_Exit(1);
}

static FILE *
openar(void)
{
	FILE *fp;
	char magic[SARMAG+1];

	if ((fp = fopen(arfile,"r+b")) == NULL) {
		if (!cflag)
			fprintf(stderr, "ar: creating %s\n", arfile);
		if ((fp = fopen(arfile, "w+b")) == NULL)
			error("opening archive: %s", errstr());
		fputs(ARMAG, fp);
		if (fflush(fp) == EOF)
			error("writing magic number: %s", errstr());
	} else {
		if (fgets(magic, sizeof(magic), fp) == NULL)
			error("error reading magic number: %s", errstr());
		if (strcmp(magic, ARMAG))
			error("invalid magic number '%s'", magic);
	}
	return fp;
}

static void
archive(char *pname, FILE *to, char letter)
{
	int c;
	FILE *from;
	char mtime[13], *fname;
	struct fprop prop;

	fname = canonical(pname);

	if (vflag)
		printf("%c - %s\n", letter, fname);
	if ((from = fopen(pname, "rb")) == NULL)
		error("opening member '%s': %s", pname, errstr());
	if (getstat(pname, &prop) < 0)
		error("error getting '%s' attributes", pname);

	strftime(mtime, sizeof(mtime), "%s", gmtime(&prop.time));
	fprintf(to,
	        "%-16.16s%-12s%-6u%-6u%-8lo%-10ld`\n",
	        fname,
	        mtime,
	        prop.uid,
	        prop.gid,
	        prop.mode,
	        prop.size);

	while ((c = getc(from)) != EOF)
		putc(c, to);
	if (prop.size & 1)
		putc('\n', to);
	if (ferror(from))
		error("reading input '%s': %s", pname, errstr());
	fclose(from);
}

static void
append(FILE *fp, char *argv[])
{
	char *fname;

	if (fseek(fp, 0, SEEK_END) == EOF)
		error("seeking archive: %s", errstr());

	for ( ; fname = *argv; ++argv) {
		*argv = NULL;
		archive(fname, fp, 'a');
	}

	if (fclose(fp) == EOF)
		error("error writing archive: %s", errstr());
}

static void
copy(struct member *m, struct tmp *tmp)
{
	int c;
	size_t siz = m->size;
	struct ar_hdr *hdr = &m->hdr;

	fwrite(hdr, sizeof(*hdr), 1, tmp->fp);
	if ((siz & 1) == 1)
		siz++;
	while (siz--) {
		if ((c = getc(m->src)) == EOF)
			break;
		fputc(c, tmp->fp);
	}
}

static void
letters(unsigned long val, char *s)
{
	*s++ = (val & 04) ? 'r' : '-';
	*s++ = (val & 02) ? 'w' : '-';
	*s++ = (val & 01) ? 'x' : '-';
}

static char *
perms(struct member *m)
{
	static char buf[10];

	letters(m->mode >> 6, buf);
	letters(m->mode >> 3, buf+3);
	letters(m->mode, buf +6);
	buf[9] = '\0';

	return buf;
}

static char *
inlist(char *fname, int argc, char *argv[])
{
	char *p;

	for ( ; argc-- > 0; ++argv) {
		if (*argv && !strcmp(canonical(*argv), fname)) {
			p = *argv;
			*argv = NULL;
			return p;
		}
	}
	return NULL;
}

static int
older(struct member *m, char *pname)
{
	struct fprop prop;

	if (getstat(pname, &prop) < 0)
		error("error getting '%s' attributes", pname);
	return prop.time > m->date;
}

static void
move(struct member *m, int argc, char *argv[])
{
	int where;

	if (inlist(m->fname, argc, argv)) {
		if (vflag)
			printf("m - %s\n", m->fname);
		where = INDOT;
	} else if (posname && !strcmp(posname, m->fname)) {
		where = (bflag) ? AFTER : BEFORE;
		m->cur = AFTER;
	} else {
		where = m->cur;
	}
	copy(m, &tmps[where]);
}

static void
insert(int argc, char *argv[])
{
	for (; argc-- > 0; ++argv) {
		if (*argv) {
			archive(*argv, tmps[INDOT].fp, 'a');
			*argv = NULL;
		}
	}
}

static void
update(struct member *m, int argc, char *argv[])
{
	int where;
	FILE *fp = tmps[BEFORE].fp;
	char *pname;

	if (pname = inlist(m->fname, argc, argv)) {
		if (!uflag || older(m, pname)) {
			archive(pname, tmps[m->cur].fp, 'r');
			return;
		}
	}

	if (posname && !strcmp(posname, m->fname)) {
		where = (bflag) ? AFTER : BEFORE;
		m->cur = AFTER;
	} else {
		where = m->cur;
	}
	copy(m, &tmps[where]);
}

static void
extract(struct member *m, int argc, char *argv[])
{
	int c;
	long siz;
	FILE *fp;
	struct fprop prop;
	struct ar_hdr *hdr = &m->hdr;

	if (argc > 0 && !inlist(m->fname, argc, argv))
		return;
	if (vflag)
		printf("x - %s\n", m->fname);
	siz = m->size;

	if ((fp = fopen(m->fname, "wb")) == NULL)
		goto error_file;
	while (siz-- > 0 && (c = getc(m->src)) != EOF)
		putc(c, fp);
	fflush(fp);
	if (fclose(fp) == EOF)
		goto error_file;

	prop.uid = atol(hdr->ar_uid);
	prop.gid = atol(hdr->ar_gid);
	prop.mode = m->mode;
	prop.time = totime(m->date);
	if (setstat(m->fname, &prop) < 0)
		error("%s: setting file attributes", m->fname);
	return;

error_file:
	error("error extracting file: %s", errstr());
}

static void
print(struct member *m, int argc, char *argv[])
{
	long siz;
	int c;

	if (argc > 0 && !inlist(m->fname, argc, argv))
		return;
	if (vflag)
		printf("\n<%s>\n\n", m->fname);
	siz = m->size;
	while (siz-- > 0 && (c = getc(m->src)) != EOF)
		putchar(c);
}

static void
list(struct member *m, int argc, char *argv[])
{
	time_t t;
	struct ar_hdr *hdr = &m->hdr;
	char mtime[30];

	if (argc > 0  && !inlist(m->fname, argc, argv))
		return;
	if (!vflag) {
		printf("%s\n", m->fname);
	} else {
		t = totime(m->date);
		strftime(mtime, sizeof(mtime), "%c", localtime(&t));
		printf("%s %ld/%ld\t%s %s\n",
		       perms(m),
		       atol(hdr->ar_uid),
		       atol(hdr->ar_gid),
		       mtime,
		       m->fname);
	}
}

static void
del(struct member *m, int argc, char *argv[])
{
	if (inlist(m->fname, argc, argv)) {
		if (vflag)
			printf("d - %s\n", m->fname);
		return;
	}
	copy(m, &tmps[BEFORE]);
}

static char *
getfname(struct ar_hdr *hdr)
{
	static char fname[SARNAM+1];
	char *p;

	memcpy(fname, hdr->ar_name, SARNAM);

	if (p = strchr(fname, ' '))
		*p = '\0';
	else
		fname[SARNAM] = '\0';

	return fname;
}

static long long
getnum(char *s, int size, int base)
{
	int c;
	long long val;
	char *p;
	static char digits[] = "0123456789";

	for (val = 0; size > 0; val += c) {
		--size;
		if ((c = *s++) == ' ')
			break;
		if ((p = strchr(digits, c)) == NULL)
			return -1;
		if ((c = p - digits) >= base)
			return -1;
		val *= base;
	}

	while (size > 0 && *s++ == ' ')
		--size;
	return (size == 0) ? val : -1;
}

static int
valid(struct member *m)
{
	struct ar_hdr *hdr = &m->hdr;

	m->fname = getfname(&m->hdr);
	m->size = getnum(hdr->ar_size, sizeof(hdr->ar_size), 10);
	m->mode = getnum(hdr->ar_mode, sizeof(hdr->ar_mode), 8);
	m->date = getnum(hdr->ar_date, sizeof(hdr->ar_date), 10);

	if (strncmp(hdr->ar_fmag, ARFMAG, sizeof(hdr->ar_fmag)) ||
	    m->size < 0 || m->mode < 0 || m->date < 0) {
		return 0;
	}
	return 1;
}

static void
run(FILE *fp, int argc, char *argv[],
    void (*fun)(struct member *, int argc, char *files[]))
{
	struct member m;

	m.src = fp;
	m.cur = BEFORE;

	while (fread(&m.hdr, sizeof(m.hdr), 1, fp) == 1) {
		fpos_t pos;

		if (!valid(&m))
			error("corrupted member '%s'", m.fname);
		fgetpos(fp, &pos);
		(*fun)(&m, argc, argv);
		fsetpos(fp, &pos);
		fseek(fp, m.size+1 & ~1, SEEK_CUR);
	}
	if (ferror(fp))
		error("reading members: %s", errstr());
	fclose(fp);
}

static void
merge(void)
{
	FILE *fp, *fi;
	int c, i;

	if ((fp = fopen(arfile, "wb")) == NULL)
		error("reopening archive: %s", errstr());

	fputs(ARMAG, fp);

	for (i = 0; i < 3; i++) {
		if ((fi = tmps[i].fp) == NULL)
			continue;
		fseek(fi, 0, SEEK_SET);
		while ((c = getc(fi)) != EOF)
			putc(c, fp);
		if (ferror(fi))
			error("error in temporary: %s", errstr());
	}

	if (fclose(fp) == EOF)
		error("writing archive file: %s", errstr());
}

static void
closetmp(int which)
{
	struct tmp *tmp = &tmps[which];

	if (!tmp->fp)
		return;
	if (fclose(tmp->fp) == EOF)
		error("closing temporaries: %s", errstr());
}

static void
opentmp(char *fname, int which)
{
	struct tmp *tmp = &tmps[which];

	if (lflag) {
		tmp->name = fname;
		tmp->fp = fopen(fname, "w+b");
	} else {
		tmp->fp = tmpfile();
	}

	if (tmp->fp == NULL)
		error("creating temporary: %s", errstr());
}

static void
ar(int key, char *argv[], int argc)
{
	FILE *fp;

	fp = openar();
	if (argc == 0 &&
	    (key == 'r' || key == 'd' || key == 'm' || key == 'q')) {
		if (fclose(fp) == EOF)
			error("early close of archive file: %s", errstr());
		return;
	}

	if (key == 'r' || key == 'm' || key == 'd')
		opentmp("ar.tmp1", BEFORE);
	if (key == 'r' || key == 'm') {
		opentmp("ar.tmp2", INDOT);
		opentmp("ar.tmp3", AFTER);
	}

	switch (key) {
	case 'r':
		run(fp, argc, argv, update);
		insert(argc, argv);
		merge();
		break;
	case 'm':
		run(fp, argc, argv, move);
		merge();
		break;
	case 'd':
		run(fp, argc, argv, del);
		merge();
		break;
	case 't':
		run(fp, argc, argv, list);
		break;
	case 'p':
		run(fp, argc, argv, print);
		break;
	case 'x':
		run(fp, argc, argv, extract);
		break;
	case 'q':
		append(fp, argv);
		break;
	}

	closetmp(BEFORE);
	closetmp(INDOT);
	closetmp(AFTER);

	for ( ; argc-- > 0; ++argv) {
		if (*argv)
			error("No member named '%s'", *argv);
	}
}

static void
checkfnames(int argc, char *argv[])
{
	size_t l;
	char *p;

	for ( ; argc-- > 0; ++argv) {
		p = canonical(*argv);
		l = strcspn(p, invalidchars);
		if (l > 16)
			error("file: '%s': name too long", *argv);
		if (p[l] != '\0')
			error("file: '%s': name invalid", *argv);
	}
}

static void
usage(void)
{
	fputs("ar [-drqtpmx][posname] [-vuaibcl] [posname] arfile name ...\n",
	      stderr);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int key, nkey = 0, pos = 0;

	atexit(cleanup);
	ARGBEGIN {
	case 'd':
		nkey++;
		key = 'd';
		break;
	case 'r':
		nkey++;
		key = 'r';
		break;
	case 'q':
		nkey++;
		key = 'q';
		break;
	case 't':
		nkey++;
		key = 't';
		break;
	case 'p':
		nkey++;
		key = 'p';
		break;
	case 'm':
		nkey++;
		key = 'm';
		break;
	case 'x':
		nkey++;
		key = 'x';
		break;
	case 'a':
		aflag = 1;
		pos++;
		posname = EARGF(usage());
		break;
	case 'i':
	case 'b':
		bflag = 1;
		pos++;
		posname = EARGF(usage());
		break;
	case 'v':
		vflag = 1;
		break;
	case 'c':
		cflag = 1;
		break;
	case 'l':
		lflag = 1;
		break;
	case 'u':
		uflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (nkey == 0 || nkey > 1 || pos > 1 || argc == 0 ||
	    (aflag || bflag) && !(key == 'm' || key == 'r') ||
	    cflag && !(key == 'q' || key == 'r') ||
	    uflag && key != 'r')
		usage();

	signal(SIGINT, sigfun);
#ifdef SIGQUIT
	signal(SIGQUIT, sigfun);
#endif
	signal(SIGTERM, sigfun);

	arfile = *argv;
	checkfnames(--argc, ++argv);
	ar(key, argv, argc);

	fflush(stdout);
	if (ferror(stdout))
		error("error writing to stdout: %s", errstr());

	return 0;
}
