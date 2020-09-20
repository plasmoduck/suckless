#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

Section *
getsec(Obj *obj, int *idx, Section *sec)
{
	return (*obj->ops->getsec)(obj, idx, sec);
}
