#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <scc/mach.h>

#include "ld.h"

static void
mksecs(void)
{
	int i;
	Obj *obj;
	Section sec, *sp;

	for (obj = objhead; obj; obj = obj->next) {
		for (i = 0; getsec(obj, &i, &sec); i++) {
			sp = lookupsec(sec.name);
			if (sp->type == '?') {
				sp->type = sec.type;
				sp->flags = sec.flags;
			}

			if (sp->type != sec.type || sp->flags != sec.flags) {
				error("incompatible definitions of section '%s'",
				      sp->name);
			}

			sp->size = sp->size+sp->align-1 & sp->align-1;
			sp->size += sec.size;
		}
	}
}

static void
mksegs(void)
{
	merge(&text);
	merge(&rodata);
	merge(&data);
	merge(&bss);
}

void
pass2(int argc, char *argv[])
{
	unsigned long long n;
	char *end;

	mksecs();
	mksegs();
}
