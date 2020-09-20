#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "ld.h"

/* TODO: This function must go in pass2 */
static void
rebase(Obj *obj)
{
	Symbol *aux;
	Objsym *sym;

	for (sym = obj->syms; sym; sym = sym->next) {
		switch (sym->type) {
		case 'T':
		case 'D':
		case 'B':
			/*
			 * this lookup must succeed, otherwise
			 * we have an error in our code.
			 */
			aux = lookup(sym->name);
			aux->value += obj->secs[sym->index].base;
		case 't':
		case 'd':
		case 'b':
			sym->value += obj->secs[sym->index].base;
		case 'U':
		case 'N':
		case '?':
			break;
		default:
			abort();
		}
	}
}

/*
 * rebase all the sections
 */
void
pass3(int argc, char *argv[])
{
	int i;
	Obj *obj;
	Section *sec;
	Segment *seg;

	/*
	 * TODO: deal with page aligment
	 */
	text.base = 0x100;
	rodata.base = text.base + text.size;
	data.base = rodata.base + rodata.size;
	bss.base = data.base + data.size;

	for (obj = objhead; obj; obj = obj->next) {
		for (i = 0; getsec(obj, &i, &sec); i++) {
			/* TODO: deal with symbol aligment */
			switch (sec->type) {
			case 'T':
				seg = &text;
				break;
			/* TODO: what happens with rodata? */
			case 'D':
				seg = &data;
				break;
			case 'B':
				seg = &bss;
				break;
			default:
				abort();
			}

			rebase(obj, i, seg->size);
			seg->size += sec.size;
		}
	}
}
