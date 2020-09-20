#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
strip(Obj *obj)
{
	return (*obj->ops->strip)(obj);
}
