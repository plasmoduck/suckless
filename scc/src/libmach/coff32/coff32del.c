#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

void
coff32del(Obj *obj)
{
	struct coff32 *coff = obj->data;

	if (coff) {
		free(coff->scns);
		free(coff->ents);
		free(coff->strtbl);
	}
	free(obj->data);
	obj->data = NULL;
}
