#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/mach.h>
#include <scc/scc.h>
#include <scc/ar.h>

#include "ld.h"

enum {
	OUTLIB,
	INLIB,
};

int bintype = -1;
Obj *objhead;

static int
is_needed(Obj *obj)
{
	int i;
	Symbol sym;

	for (i = 0; getsym(obj, &i, &sym); i++) {
		if (hasref(sym.name))
			return 1;
	}

	return 0;
}

static void
newsec(Section *osec, Obj *obj)
{
	int align;
	Section *sec;
	unsigned long long base;

	sec = lookupsec(osec->name);
	if (sec->type == 'U') {
		sec->type = osec->type;
		sec->base = osec->base;
		sec->size = osec->size;
		sec->flags = osec->flags;
		align = 0;
	} else {
		if (sec->type != osec->type
		|| sec->flags != osec->flags
		|| sec->align != osec->align) {
			error("incompatible definition of section %s",
			      sec->name);
			return;
		}
		align = osec->align;
		align -= sec->size & align-1;
		grow(sec, align);
	}

	rebase(obj, osec->index, sec->size);
	copy(obj, osec, sec);
}

static void
newsym(Symbol *sym, Obj *obj)
{
	int id;
	Section sec;

	if (sym->type == 'U' || islower(sym->type))
		return;

	sym = define(sym, obj);
	id = sym->section;
	getsec(obj, &id, &sec);
  	sym->value += sec.base;
}

static void
load(FILE *fp, int inlib)
{
	int t, i;
	Obj *obj;
	Section sec;
	Symbol sym;
	static Obj *last;

	if ((t = objtype(fp, NULL)) < 0) {
		error("bad format");
		return;
	}

	if (bintype != -1 && bintype != t) {
		error("not compatible object file");
		return;
	}
	bintype = t;

	if ((obj = newobj(t)) == NULL) {
		error(strerror(errno));
		return;
	}

	if (readobj(obj, fp) < 0) {
		error(strerror(errno));
		goto delete;
	}

	if (inlib && !is_needed(obj))
		goto delete;

	for (i = 0; getsec(obj, &i, &sec); i++)
		newsec(&sec, obj);

	for ( i = 0; getsym(obj, &i, &sym); i++)
		newsym(&sym, obj);

	obj->next = last;
	last = obj;
	if (!objhead)
		objhead = obj;

	return;

 delete:
	delobj(obj);
	return;
}

static void
scanindex(FILE *fp)
{
	int t, added;
	long n, i, *offs;
	char **names;
	Symbol *sym;

	if (getindex(bintype, &n, &names, &offs, fp) < 0) {
		error("corrupted index");
		return;
	}

	for (added = 0; moreundef(); added = 0) {
		for (i = 0; i < n; i++) {
			if (!hasref(names[i]))
				continue;

			if (fseek(fp, offs[i], SEEK_SET) == EOF) {
				error(strerror(errno));
				goto clean;
			}

			load(fp, OUTLIB);
			added = 1;
		}

		if (!added)
			break;
	}
clean:
	for (i = 0; i < n; i++)
		free(names[i]);
	free(names);
	free(offs);
}

void
scanlib(FILE *fp)
{
	long cur, off;
	char memb[SARNAM+1];

	if (bintype == -1) {
		error("an object file is needed before any library");
		return;
	}

	cur = ftell(fp);
	if ((off = armember(fp, memb)) < 0)
		goto corrupted;

	if (strcmp(memb, "/") == 0 || strcmp(memb, "__.SYMDEF") == 0) {
		scanindex(fp);
		return;
	}

	fseek(fp, cur, SEEK_SET);
	for (;;) {
		cur = ftell(fp);
		off = armember(fp, memb);
		switch (off) {
		case -1:
			goto corrupted;
		case 0:
			return;
		default:
			membname = memb;
			if (objtype(fp, NULL) != -1)
				load(fp, INLIB);
			membname = NULL;
			fseek(fp, cur, SEEK_SET);
			fseek(fp, off, SEEK_CUR);
			break;
		}
	}

corrupted:
	error(strerror(errno));
	error("library corrupted");
}

static FILE *
openfile(char *name)
{
	size_t pathlen, len;
	FILE *fp;
	char **bp;
	char libname[FILENAME_MAX];
	static char buffer[FILENAME_MAX];

	filename = name;
	membname = NULL;
	if (name[0] != '-' || name[1] != 'l') {
		if ((fp = fopen(name, "rb")) == NULL)
			error(strerror(errno));
		return fp;
	}

	len = strlen(name+2) + 3;
	if (len > FILENAME_MAX-1) {
		error("library name too long");
		return NULL;
	}
	strcat(strcpy(buffer, "lib"), name+2);

	filename = buffer;
	if ((fp = fopen(buffer, "rb")) != NULL)
		return fp;

	for (bp = libpaths; *bp; ++bp) {
		pathlen = strlen(*bp);
		if (pathlen + len > FILENAME_MAX-1)
			continue;
		memcpy(libname, *bp, pathlen);
		memcpy(libname+pathlen+1, buffer, len);
		buffer[pathlen] = '/';

		if ((fp = fopen(libname, "rb")) != NULL)
			return fp;
	}

	error("not found");
	return NULL;
}

static void
process(char *name)
{
	int t;
	FILE *fp;

	if ((fp = openfile(name)) == NULL)
		return;

	if (archive(fp))
		scanlib(fp);
	else
		load(fp, OUTLIB);

	fclose(fp);
}

/*
 * Get the list of object files that are going to be linked
 */
void
pass1(int argc, char *argv[])
{
	char **av, *ap;

	for (av = argv+1; *av; ++av) {
		if (av[0][0] != '-') {
			process(*av);
			continue;
		}
		for (ap = &av[0][1]; *ap; ++ap) {
			switch (*ap) {
			case 'l':
				process(nextarg(&ap, &av));
				break;
			case 'u':
				lookupsym(nextarg(&ap, &av));
				break;
			}
		}
	}

	if (moreundef()) {
		listundef();
		exit(EXIT_FAILURE);
	}
}
