#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
objtype(FILE *fp, char **name)
{
	int n, i;
	fpos_t pos;
	Objops **opsp, *ops;
	unsigned char buf[NBYTES];

	fgetpos(fp, &pos);
	n = fread(buf, NBYTES, 1, fp);
	fsetpos(fp, &pos);

	if (n != 1 || ferror(fp))
		return -1;

	for (opsp = objops; ops = *opsp; ++opsp) {
		if ((*ops->probe)(buf, name) < 0)
			continue;
		return opsp - objops;
	}

	return -1;
}
