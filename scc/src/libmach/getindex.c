#include <errno.h>
#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
getindex(int type, long *nsyms, char ***names, long **offs, FILE *fp)
{
	int fmt;

	fmt = FORMAT(type);
	if (fmt >= NFORMATS) {
		errno = ERANGE;
		return -1;
	}

	return (*objops[fmt]->getidx)(nsyms, names, offs, fp);
}

