#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

Map *
loadmap(Obj *obj, FILE *fp)
{
	return (*obj->ops->loadmap)(obj, fp);
}
