#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

int
coff32setidx(long nsymbols, char *names[], long offs[], FILE *fp)
{
	return coff32xsetidx(BIG_ENDIAN, nsymbols, names, offs, fp);
}
