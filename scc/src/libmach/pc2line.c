#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
pc2line(Obj *obj, unsigned long long pc, char *fname, int *ln)
{
	return (*obj->ops->pc2line)(obj, pc, fname, ln);
}
