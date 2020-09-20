#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

static int
typeof(Coff32 *coff, SYMENT *ent)
{
	int c;
	SCNHDR *scn;
	long flags;

	switch (ent->n_scnum) {
	case N_DEBUG:
		c = 'N';
		break;
	case N_ABS:
		c = 'a';
		break;
	case N_UNDEF:
		c = (ent->n_value != 0) ? 'C' : 'U';
		break;
	default:
		scn = &coff->scns[ent->n_scnum-1];
		flags = scn->s_flags;
		if (flags & STYP_TEXT)
			c = 't';
		else if (flags & STYP_DATA)
			c = 'd';
		else if (flags & STYP_BSS)
			c = 'b';
		else
			c = '?';
		break;
	}

	if (ent->n_sclass == C_EXT)
		c = toupper(c);

	return c;
}

static char *
symname(Coff32 *coff, SYMENT *ent)
{
	if (ent->n_zeroes != 0)
		return ent->n_name;

	return &coff->strtbl[ent->n_offset];
}

Symbol *
coff32getsym(Obj *obj, int *idx, Symbol *sym)
{
	long n = *idx;
	SYMENT *ent;
	Coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;

	if ((hdr->f_flags & F_SYMS) != 0 || n >= coff->hdr.f_nsyms)
		return NULL;

	ent = &coff->ents[n];
	sym->name = symname(coff, ent);
	sym->type = typeof(coff, ent);
	sym->value = ent->n_value;
	sym->size = (sym->type == 'C') ? ent->n_value : 0;
	sym->index = n;
	*idx += ent->n_numaux;

	return sym;
}
