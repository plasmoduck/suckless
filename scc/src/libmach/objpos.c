#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
objpos(Obj *obj, FILE *fp, long pos)
{
	if (fseek(fp, obj->pos, SEEK_SET) == EOF)
		return 0;
	if (fseek(fp, pos, SEEK_CUR) < 0)
		return 0;
	return 1;
}
