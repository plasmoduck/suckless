#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

Map *
coff32loadmap(Obj *obj, FILE *fp)
{
	long i;
	Map *map;
	long nsec;
	SCNHDR *scn;
	struct coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;

	nsec = hdr->f_nscns;
	if ((map = newmap(nsec, fp)) == NULL)
		return NULL;

	for (scn = coff->scns; nsec--; ++scn) {
		unsigned long o = obj->pos + scn->s_scnptr;
		unsigned long long b = scn->s_paddr;
		unsigned long long e = b + scn->s_size;

		setmap(map, scn->s_name, fp, b, e, o);
	}

	return map;
}
