#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

int
coff32getidx(long *nsyms, char ***namep, long **offsp, FILE *fp)
{
	return coff32xgetidx(BIG_ENDIAN, nsyms, namep, offsp, fp);
}
