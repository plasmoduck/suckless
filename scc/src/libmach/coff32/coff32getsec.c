#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

Section *
coff32getsec(Obj *obj, int *idx, Section *sec)
{
	long n = *idx;
	int type;
	unsigned sflags;
	unsigned long flags;
	SCNHDR *scn;
	Coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;

	if (n >= hdr->f_nscns)
		return NULL;

	scn = &coff->scns[n];
	flags = scn->s_flags;

	if (flags & STYP_TEXT) {
		type = 'T';
		sflags = SALLOC | SRELOC | SLOAD | SEXEC | SREAD;
		if (flags & STYP_NOLOAD)
			sflags |= SSHARED;
	} else if (flags & STYP_DATA) {
		type = 'D';
		sflags = SALLOC | SRELOC | SLOAD | SWRITE | SREAD;
		if (flags & STYP_NOLOAD)
			sflags |= SSHARED;
	} else if (flags & STYP_BSS) {
		type = 'B';
		sflags = SALLOC | SREAD | SWRITE;
	} else if (flags & STYP_INFO) {
		type = 'N';
		sflags = 0;
	} else if (flags & STYP_LIB) {
		type = 'T';
		sflags = SRELOC;
	} else if (flags & STYP_DSECT) {
		type = 'D';
		sflags = SRELOC;
	} else if (flags & STYP_PAD) {
		type = 'D';
		sflags = SLOAD;
	} else {
		type = 'D';  /* We assume that STYP_REG is data */
		sflags = SALLOC | SRELOC | SLOAD | SWRITE | SREAD;
	}

	if (flags & STYP_NOLOAD)
		sflags &= ~SLOAD;

	sec->name = scn->s_name;
	sec->index = n;
	sec->size = scn->s_size;
	sec->base = 0; /* TODO: Check what is the actual value */
	sec->type = type;
	sec->flags = sflags;
	sec->align = 4; /* TODO: Check how align is defined in coff */

	return sec;
}
