#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

Symbol *
getsym(Obj *obj, int *index, Symbol *sym)
{
	return (*obj->ops->getsym)(obj, index, sym);
}
