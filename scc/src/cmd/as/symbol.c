#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>
#include "as.h"

#define HASHSIZ 64
#define NALLOC  10

Section *cursec, *seclist;
Section *sabs, *sbss, *sdata, *stext;
Symbol *linesym, *symlist;
int pass;

static Symbol *hashtbl[HASHSIZ], *symlast;
static Alloc *tmpalloc;


#ifndef NDEBUG
void
dumpstab(char *msg)
{
	Symbol **bp, *sym;

	fprintf(stderr, "%s\n", msg);
	for (bp = hashtbl; bp < &hashtbl[HASHSIZ]; ++bp) {
		if (*bp == NULL)
			continue;

		fprintf(stderr, "[%d]", (int) (bp - hashtbl));
		for (sym = *bp; sym; sym = sym->hash) {
			fprintf(stderr, " -> %s:%0X:%0X",
			       sym->name.buf, sym->flags, sym->value);
		}
		putc('\n', stderr);
	}
}
#endif

Symbol *
lookup(char *name)
{
	unsigned h;
	Symbol *sym, **list;
	int c, symtype;
	char *t;

	h = genhash(name) & HASHSIZ-1;

	c = toupper(*name);
	list = &hashtbl[h];
	for (sym = *list; sym; sym = sym->hash) {
		t = sym->name.buf;
		if (c == toupper(*t) && !casecmp(t, name))
			return sym;
	}

	sym = xmalloc(sizeof(*sym));
	sym->name = newstring(name);
	sym->flags = 0;
	sym->size = sym->value = 0;
	sym->section = cursec;
	sym->hash = *list;
	sym->next = NULL;

	*list = sym;
	if (symlast)
		symlast->next = sym;
	symlast = sym;
	if (!symlist)
		symlist = sym;

	return sym;
}

Symbol *
deflabel(char *name)
{
	static Symbol *cursym;
	Symbol *sym;
	char label[MAXSYM+1];

	if (*name == '.') {
		int r;

		if (!cursym) {
			error("local label '%s' without global label", name);
			return NULL;
		}
		r = snprintf(label, sizeof(label),
		             "%s%s",
		             cursym->name.buf, name);
		if (r == sizeof(label)) {
			error("local label '%s' in '%s' produces too long symbol",
			      name, cursym->name.buf);
			return NULL;
		}
		name = label;
	}

	sym = lookup(name);
	if (pass == 1 && (sym->flags & FDEF))
		error("redefinition of label '%s'", name);
	if (cursec->flags & SABS)
		sym->flags |= FABS;
	sym->flags |= FDEF;
	sym->value = cursec->curpc;
	sym->section = cursec;

	if (*name != '.')
		cursym = sym;
	return sym;
}

int
toobig(Node *np, int type)
{
	unsigned long long val = np->sym->value;

	switch (type) {
	case AIMM2:
		return val > 3;
	case AIMM3:
		return val > 7;
	case AIMM5:
		return val > 0x1F;
	case AIMM8:
		return val > 0xFF;
	case AIMM16:
		return val > 0xFFFF;
	case AIMM32:
		return val > 0xFFFFFFFF;
	case AIMM64:
		return 1;
	default:
		abort();
	}
}

static void
incpc(int siz)
{
	TUINT pc, curpc;

	pc = cursec->pc;
	curpc = cursec->curpc;

	cursec->curpc += siz;
	cursec->pc += siz;

	if (pass == 2)
		return;

	if (cursec->pc > cursec->max)
		cursec->max = cursec->pc;

	if (pc > cursec->pc ||
	    curpc > cursec->curpc ||
	    cursec->curpc > maxaddr ||
	    cursec->pc > maxaddr) {
		die("as: address overflow in section '%s'");
	}
}

static int
secflags(char *attr)
{
	int c, flags;

	if (!attr)
		return 0;

	for (flags = 0; c = *attr++; ) {
		switch (c) {
		case 'w':
			flags |= SWRITE;
			break;
		case 'r':
			flags |= SREAD;
			break;
		case 'x':
			flags |= SEXEC;
			break;
		case 'f':
			flags |= SFILE;
			break;
		case 'l':
			flags |= SLOAD;
			break;
		case 'a':
			flags |= SABS;
			break;
		}
	}

	return flags;
}

Section *
setsec(char *name, char *attr)
{
	Section *sec;
	Symbol *sym;

	cursec = NULL;
	sym = lookup(name);
	if (sym->flags & ~FSECT)
		error("invalid section name '%s'", name);

	if ((sec = sym->section) == NULL) {
		sec = xmalloc(sizeof(*sec));
		sec->mem = NULL;
		sec->sym = sym;
		sec->base = sec->max = sec->pc = sec->curpc = 0;
		sec->next = seclist;
		sec->flags = 0;
		sec->fill = 0;
		sec->aligment = 0;
		sec->next = seclist;
		seclist = sec;

		sym->section = sec;
		sym->flags = FSECT;
	}
	sec->flags |= secflags(attr);

	return cursec = sec;
}

void
isecs(void)
{
	sabs = setsec(".abs", "rwx");
	sbss = setsec(".bss", "rwf");
	sdata = setsec(".data", "rw");
	stext = setsec(".text", "rx");
}

void
cleansecs(void)
{
	Section *sec;
	TUINT siz;

	for (sec = seclist; sec; sec = sec->next) {
		sec->curpc = sec->pc = sec->base;
		if (pass == 1 || sec->flags & SFILE)
			continue;

		siz = sec->max - sec->base;
		if (siz > SIZE_MAX)
			die("as: out of memory");
		sec->mem = xmalloc(sec->max - sec->base);
	}
	cursec = stext;
}

void
emit(char *bytes, int n)
{
	if (cursec->mem) {
		size_t len = cursec->pc - cursec->base;
		memcpy(&cursec->mem[len], bytes, n);
	}
	incpc(n);
}

Symbol *
tmpsym(TUINT val)
{
	Symbol *sym;

	if (!tmpalloc)
		tmpalloc = alloc(sizeof(*sym), NALLOC);
	sym = new(tmpalloc);
	sym->value = val;
	sym->section = NULL;
	sym->flags = FABS;

	return sym;
}

void
killtmp(void)
{
	if (!tmpalloc)
		return;
	dealloc(tmpalloc);
	tmpalloc = NULL;
}

String
newstring(char *s)
{
	size_t len = strlen(s) + 1;
	String str;

	str.offset = 0;
	str.buf = xmalloc(len);
	memcpy(str.buf, s, len);
	return str;
}
