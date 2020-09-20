#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <scc/ar.h>
#include <scc/arg.h>
#include <scc/mach.h>
#include <scc/scc.h>

#include "sys.h"

#define NR_SYMDEF 32

typedef struct symdef Symdef;

struct symdef {
	char *name;
	int type;
	long offset;
	Symdef *hash, *next;
};

static char *namidx;
static long nsymbols;
static int status, artype, nolib;
static char *filename, *membname;
static Symdef *htab[NR_SYMDEF], *head;
static long offset;
char *argv0;

static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "ranlib: %s: ", filename);
	if (membname)
		fprintf(stderr, "%s: ", membname);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);

	status = EXIT_FAILURE;
}

Symdef *
lookup(char *name)
{
	unsigned h;
	Symdef *dp;
	char *s;
	size_t len;

	h = genhash(name) % NR_SYMDEF;

	for (dp = htab[h]; dp; dp = dp->next) {
		if (!strcmp(dp->name, name))
			return dp;
	}

	len = strlen(name) + 1;
	dp = malloc(sizeof(*dp));
	s = malloc(len);
	if (!dp || !s) {
		free(s);
		free(dp);
		return NULL;
	}

	nsymbols++;
	dp->name = s;
	memcpy(dp->name, name, len);
	dp->type = 'U';
	dp->offset = -1;
	dp->hash = htab[h];
	htab[h] = dp;
	dp->next = head;
	head = dp;

	return dp;
}

static int
newsymbol(Symbol *sym)
{
	Symdef *np;

	if (!isupper(sym->type) || sym->type == 'N')
		return 1;

	if ((np = lookup(sym->name)) == NULL) {
		error(strerror(errno));
		return 0;
	}

	switch (np->type) {
	case 'C':
		if (sym->type == 'C')
			break;
	case 'U':
		np->type = sym->type;
		np->offset = offset;
		break;
	default:
		if (sym->type != 'C') {
			error("multiple definitions of '%s'", sym->name);
			return 0;
		}
	}

	return 1;
}

static void
freehash(void)
{
	Symdef **npp, *next, *np;

	for (npp = htab; npp < &htab[NR_SYMDEF]; npp++)
		*npp = NULL;

	for (np = head; np; np = next) {
		next = np->next;
		free(np->name);
		free(np);
	}

	head = NULL;
}

static int
newmember(FILE *fp)
{
	int i,t, ret = 0;
	Obj *obj;
	Symbol sym;

	offset = ftell(fp);

	if (offset == EOF) {
		error(strerror(errno));
		return 0;
	}

	t = objtype(fp, NULL);
	if (t == -1 || artype != -1 && artype != t) {
		nolib = 1;
		return 0;
	}
	artype = t;

	if ((obj = newobj(t)) == NULL) {
		error(strerror(errno));
		return 0;
	}
	namidx = obj->index;

	if (readobj(obj, fp) < 0) {
		error(strerror(errno));
		goto error;
	}

	for (i = 0; getsym(obj, &i, &sym); i++) {
		if (!newsymbol(&sym))
			goto error;
	}

	ret = 1;

error:
	delobj(obj);
	return ret;
}

static int
readsyms(FILE *fp)
{
	long cur, off;
	char memb[SARNAM+1];

	nolib = 0;
	artype = -1;
	nsymbols = 0;

	if (!archive(fp)) {
		error("file format not recognized");
		return 0;
	}

	cur = ftell(fp);
	if ((off = armember(fp, memb)) < 0)
		goto corrupted;

	if (strcmp(memb, "/") == 0 || strcmp(memb, "__.SYMDEF") == 0)
		cur = ftell(fp) + off;

	fseek(fp, cur, SEEK_SET);
	for (;;) {
		cur = ftell(fp);
		off = armember(fp, memb);
		switch (off) {
		case -1:
			goto corrupted;
		case 0:
			return (nolib || nsymbols == 0) ? -1 : 0;
		default:
			membname = memb;
			if (objtype(fp, NULL) != -1)
				newmember(fp);
			membname = NULL;
			fseek(fp, cur, SEEK_SET);
			fseek(fp, off, SEEK_CUR);
			break;
		}
	}

corrupted:
	error(strerror(errno));
	error("library corrupted");
	return 0;
}

static void
merge(FILE *to, struct fprop *prop, FILE *lib, FILE *idx)
{
	int c;
	char mtime[13];
	struct ar_hdr first;

	rewind(lib);
	rewind(idx);
	fseek(lib, SARMAG, SEEK_SET);

	if (fread(&first, sizeof(first), 1, lib) != 1)
		return;

	if (!strncmp(first.ar_name, namidx, SARNAM))
		fseek(lib, atol(first.ar_size), SEEK_CUR);
	else
		fseek(lib, SARMAG, SEEK_SET);

	fwrite(ARMAG, SARMAG, 1, to);

        strftime(mtime, sizeof(mtime), "%s", gmtime(&prop->time));
        fprintf(to,
                "%-16.16s%-12s%-6u%-6u%-8lo%-10ld`\n",
                namidx,
                mtime,
                prop->uid,
                prop->gid,
                prop->mode,
                prop->size);

	while ((c = getc(idx)) != EOF)
		putc(c, to);
	if (prop->size & 1)
		putc('\n', to);

	while ((c = getc(lib)) != EOF)
		putc(c, to);

	fflush(to);
}

static void
ranlib(char *fname)
{
	size_t r;
	long *offs, i;
	char **names;
	FILE *fp, *idx, *out;
	Symdef *dp;
	struct fprop prop;
	char tmpname[FILENAME_MAX];

	filename = fname;
	if ((fp = fopen(fname, "rb")) == NULL) {
		error(strerror(errno));
		return;
	}

	if (readsyms(fp) <0)
		goto err2;

	if ((idx = tmpfile()) == NULL) {
		error(strerror(errno));
		goto err2;
	}

	offs = malloc(sizeof(long) * nsymbols);
	names = malloc(sizeof(*names) * nsymbols);
	if (!offs || !names) {
		error(strerror(errno));
		goto err3;
	}

	for (dp = head, i = 0; i < nsymbols; dp = dp->next, i++) {
		offs[i] = dp->offset;
		names[i] = dp->name;
	}

	if (setindex(artype, nsymbols, names, offs, idx) < 0) {
		error(strerror(errno));
		goto err3;
	}

	if (getstat(fname, &prop) < 0) {
		error(strerror(errno));
		goto err3;
	}
	prop.size = ftell(idx);
	prop.time = time(NULL);

	r = snprintf(tmpname, sizeof(tmpname), "%s.tmp", fname);
	if (r >= sizeof(tmpname)) {
		error("too long temporary name");
		goto err3;
	}

	if ((out = fopen(tmpname, "wb")) == NULL) {
		error(strerror(errno));
		goto err3;
	}

	merge(out, &prop, fp, idx);
	if (ferror(out) || ferror(fp) || ferror(idx)) {
		error(strerror(errno));
		fclose(out);
		goto err4;
	}

	fclose(out);
	if (rename(tmpname, fname) == EOF) {
		error(strerror(errno));
		goto err4;
	}

err4:
	remove(tmpname);
err3:
	free(offs);
	free(names);
	fclose(idx);
err2:
	freehash();
err1:
	fclose(fp);
}

static void
usage(void)
{
	fputs("usage: ranlib [-t] file...\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 't':
		break;
	default:
		usage();
	} ARGEND

	if (argc == 0)
		usage();

	for (; *argv; ++argv)
		ranlib(*argv);

	return status;
}
