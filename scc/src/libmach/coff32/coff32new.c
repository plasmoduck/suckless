#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

int
coff32new(Obj *obj)
{
	struct coff32 *coff;

	if ((coff = calloc(1, sizeof(*coff))) == NULL)
		return -1;
	obj->data = coff;
	obj->index = "/";
	return 0;
}
