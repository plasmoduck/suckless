#include <errno.h>
#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
setindex(int type, long nsyms, char **names, long *offs, FILE *fp)
{
	int fmt;

	fmt = FORMAT(type);
	if (fmt >= NFORMATS) {
		errno = ERANGE;
		return -1;
	}

	return (*objops[fmt]->setidx)(nsyms, names, offs, fp);
}

