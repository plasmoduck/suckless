#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <scc/mach.h>

#include "libmach.h"

Obj *
newobj(int type)
{
	Obj *obj;
	int fmt;

	fmt = FORMAT(type);
	if (fmt >= NFORMATS) {
		errno = ERANGE;
		return NULL;
	}

	if ((obj = malloc(sizeof(*obj))) == NULL)
		return NULL;

	obj->type = type;
	obj->ops = objops[fmt];
	obj->next = NULL;
	if ((*obj->ops->new)(obj) < 0) {
		free(obj);
		return NULL;
	}

	return obj;
}
