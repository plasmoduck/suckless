#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

Objops *objops[] = {
	[COFF32] = &coff32,
	[NFORMATS] = NULL,
};
