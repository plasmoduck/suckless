#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "libmach.h"

void
delobj(Obj *obj)
{
	(*obj->ops->del)(obj);
	free(obj);
}