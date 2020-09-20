#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
writeobj(Obj *obj, Map *map, FILE *fp)
{
	return (obj->ops->write)(obj, map, fp);
}
