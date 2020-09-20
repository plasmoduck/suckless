#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

static struct arch archs[] = {
	"coff32-i386", "\x4c\x01", OBJ(COFF32, ARCH386, LITTLE_ENDIAN),
	"coff32-z80", "\x5a\x80", OBJ(COFF32, ARCHZ80, LITTLE_ENDIAN),
	NULL,
};

int
coff32probe(unsigned char *buf, char **name)
{
	struct arch *ap;

	for (ap = archs; ap->name; ap++) {
		if (ap->magic[0] == buf[0] && ap->magic[1] == buf[1]) {
			if (name)
				*name = ap->name;
			return ap->type;
		}
	}
	return -1;
}
