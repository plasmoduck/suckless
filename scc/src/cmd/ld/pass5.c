#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "ld.h"

extern int bintype;
extern Symbol defhead;

/*
        char *name;
        int type;
        int sect;
        unsigned long long size;
        unsigned long long value;
*/

void
pass5(int argc, char *argv[])
{
	Obj *obj;
	Symbol *sym;
	Segment **segp;
	Objsym *osym;
	FILE *fp;
	static Segment *segv[] = {
		&text,
		&data,
		&bss,
		/* TODO: debug, */
		NULL,
	};

	obj = objnew(bintype);

	for (segp = segv; *segp; segp++)
		/* objaddseg(obj, *segp) */ ;

	for (sym = defhead.next; sym != &defhead; sym = sym->next) {
		osym = objlookup(obj, sym->name, 1);
		osym->size = sym->size;
		osym->value = sym->value;
	}

	/* TODO: write relocations */
	/* TODO: write line information */

	fp = fopen(output, "wb");
	(obj->ops->write)(obj, fp);
}
