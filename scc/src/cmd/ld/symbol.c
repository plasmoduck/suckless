#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/mach.h>
#include <scc/scc.h>

#include "ld.h"

#define NR_SYMBOL 128

/*
 * struct symtab has a Symbol as first field because
 * the code is going to cast from the symbols to the tab.
 */
struct symtab {
	Symbol sym;
	Obj *where;
	struct symtab *hash;
	struct symtab *next, *prev;
};

static struct symtab *symtab[NR_SYMBOL];
static struct symtab undef = {.next = &undef, .prev = &undef};
static struct symtab def = {.next = &def, .prev = &def};
static struct symtab common = {.next = &common, .prev = &common};

static Symbol *
unlinksym(Symbol *sym)
{
	struct symtab *sp = (struct symtab *) sym;

	sp->next->prev = sp->prev;
	sp->prev->next = sp->next;

	return sym;
}

static Symbol *
linksym(struct symtab *lst, Symbol *sym)
{
	struct symtab *sp = (struct symtab *) sym;

	sp->next = lst;
	sp->prev = lst->prev;
	lst->prev->next = sp;
	lst->prev = sp;

	return sym;
}

int
hasref(char *name)
{
	unsigned h;
	struct symtab *sp;

	h = genhash(name) % NR_SYMBOL;
	for (sp = symtab[h]; sp; sp = sp->hash) {
		if (!strcmp(name, sp->sym.name))
			return sp->sym.type == 'U';
	}
	return 0;
}

Symbol *
lookupsym(char *name)
{
	unsigned h;
	size_t len;
	char *s;
	Symbol *sym;
	struct symtab *sp;

	h = genhash(name) % NR_SYMBOL;
	for (sp = symtab[h]; sp; sp = sp->hash) {
		if (!strcmp(name, sp->sym.name))
			return &sp->sym;
	}

	len = strlen(name) + 1;
	s = malloc(len);
	sp = malloc(sizeof(*sp));
	if (!s  || !sp) {
		error(strerror(errno));
		exit(EXIT_FAILURE);
	}

	sym = &sp->sym;
	sym->name = memcpy(s, name, len);
	sym->value = 0;
	sym->size = 0;
	sym->index = 0;
	sym->type = 'U';
	sp->where = NULL;
	sp->hash = symtab[h];
	symtab[h] = sp;

	return linksym(&undef, sym);;
}


int
moreundef(void)
{
	return undef.next != &undef;
}

void
listundef(void)
{
	struct symtab *sp;

	for (sp = undef.next; sp != &undef; sp = sp->next)
		error("ld: symbol '%s' not defined", sp->sym.name);
}

Symbol *
define(Symbol *osym, Obj *obj)
{
	struct symtab *lst;
	Symbol *sym = lookupsym(osym->name);
	struct symtab *sp = (struct symtab *) sym;

	assert(osym->type != 'U');
	sp->where = obj;

	switch (sym->type) {
	case 'U':
		sym->value = osym->value;
		sym->size = osym->size;
		lst = (osym->type == 'C') ? &common : &def;
		linksym(lst, unlinksym(sym));
		break;
	case 'C':
		if (osym->type != 'C') {
			sym->size = osym->size;
			sym->value = osym->size;
			linksym(&def, unlinksym(sym));
		} else  if (sym->size < osym->size) {
			sym->value = osym->value;
			sym->size = osym->size;
		}
		break;
	defaul:
		error("%s: symbol redefined", sym->name);
		break;
	}

	return sym;
}

#ifndef NDEBUG
void
debugsym(void)
{
	struct symtab **spp, *sp;
	Symbol*sym;

	fputs("Symbols:\n", stderr);
	for (spp = symtab; spp < &symtab[NR_SYMBOL]; spp++) {
		for (sp = *spp; sp; sp = sp->hash) {
			sym = &sp->sym;
			fprintf(stderr,
			        "sym: %s (%#llx)\n",
			        sym->name,
			        sym->value);
		}
	}
}
#endif
