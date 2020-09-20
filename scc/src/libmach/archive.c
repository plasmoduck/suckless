#include <stdio.h>
#include <string.h>

#include <scc/ar.h>
#include <scc/mach.h>
#include "libmach.h"

int
archive(FILE *fp)
{
	int n;
	fpos_t pos;
	char magic[SARMAG];

	fgetpos(fp, &pos);

	n = fread(magic, SARMAG, 1, fp);
	if (n == 1 && strncmp(magic, ARMAG, SARMAG) == 0)
		return 1;

	fsetpos(fp, &pos);

	return 0;
}
