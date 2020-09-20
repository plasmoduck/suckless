#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

int
coff32strip(Obj *obj)
{
	int i;
	SCNHDR *scn;
	struct coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;

	for (i = 0; i < hdr->f_nscns; i++) {
		scn = &coff->scns[i];
		scn->s_nrelloc = 0;
		scn->s_relptr = 0;
		scn->s_nlnno = 0;
		scn->s_lnnoptr = 0;
	}

	hdr->f_nsyms = 0;
	hdr->f_symptr = 0;
	hdr->f_flags |= F_RELFLG | F_LMNO | F_SYMS;

	free(coff->ents);
	free(coff->rels);
	free(coff->lines);

	coff->ents = NULL;
	coff->rels = NULL;
	coff->lines = NULL;

	return 0;
}
