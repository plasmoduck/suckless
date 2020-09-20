#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
readobj(Obj *obj, FILE *fp)
{
	long off;

	if ((off = ftell(fp)) == EOF)
		return -1;
	obj->pos = off;

	return (*obj->ops->read)(obj, fp);
}
