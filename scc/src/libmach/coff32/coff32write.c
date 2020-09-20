#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/mach.h>

#include "../libmach.h"
#include "coff32.h"

static void
pack_hdr(int order, unsigned char *buf, FILHDR *hdr)
{
	int n;

	n = pack(order,
	         buf,
	         "sslllss",
	         hdr->f_magic,
	         hdr->f_nscns,
	         hdr->f_timdat,
	         hdr->f_symptr,
	         hdr->f_nsyms,
	         hdr->f_opthdr,
	         hdr->f_flags);
	assert(n == FILHSZ);
}

static void
pack_scn(int order, unsigned char *buf, SCNHDR *scn)
{
	int n;

	n = pack(order,
	         buf,
	         "'8llllllssl",
	         scn->s_name,
	         scn->s_paddr,
	         scn->s_vaddr,
	         scn->s_size,
	         scn->s_scnptr,
	         scn->s_relptr,
	         scn->s_lnnoptr,
	         scn->s_nrelloc,
	         scn->s_nlnno,
	         scn->s_flags);
	assert(n == SCNHSZ);
}

static void
pack_ent(int order, unsigned char *buf, SYMENT *ent)
{
	int n;
	char *s;

	/* TODO: What happens with the union? */

	n = pack(order,
	         buf,
	         "'8lsscc",
	         ent->n_name,
	         ent->n_value,
	         ent->n_scnum,
	         ent->n_type,
	         ent->n_sclass,
	         ent->n_numaux);
	assert(n == SYMESZ);
}

static void
pack_aout(int order, unsigned char *buf, AOUTHDR *aout)
{
	int n;

	n = pack(order,
	         buf,
	         "ssllllll",
	         aout->magic,
	         aout->vstamp,
	         aout->tsize,
	         aout->dsize,
	         aout->bsize,
	         aout->entry,
	         aout->text_start,
	         aout->data_start);
	assert(n == AOUTSZ);
}

static void
pack_reloc(int order, unsigned char *buf, RELOC *rel)
{
	int n;

	n = pack(order,
	         buf,
	         "lls",
	         rel->r_vaddr,
	         rel->r_symndx,
	         rel->r_type);
	assert(n == RELSZ);
}

static void
pack_line(int order, unsigned char *buf, LINENO *lp)
{
	int n;

	n = pack(order,
	         buf,
	         "lls",
	         lp->l_symndx,
	         lp->l_paddr,
	         lp->l_lnno);
	assert(n == LINESZ);
}

static int
writehdr(Obj *obj, FILE *fp)
{
	FILHDR *hdr;
	struct coff32 *coff;
	unsigned char buf[FILHSZ];

	coff  = obj->data;
	hdr = &coff->hdr;

	pack_hdr(ORDER(obj->type), buf, hdr);
	if (fwrite(buf, FILHSZ, 1, fp) != 1)
		return 0;

	return 1;
}

static int
writescns(Obj *obj, FILE *fp)
{
	int i;
	SCNHDR *scn;
	FILHDR *hdr;
	struct coff32 *coff;
	unsigned char buf[SCNHSZ];

	coff  = obj->data;
	hdr = &coff->hdr;

	for (i = 0; i < hdr->f_nscns; i++) {
		scn = &coff->scns[i];
		pack_scn(ORDER(obj->type), buf, scn);
		if (fwrite(buf, SCNHSZ, 1, fp) != 1)
			return 0;
	}

	return 1;
}

static int
writeents(Obj *obj, FILE *fp)
{
	long i, len, strsiz;
	char *strtbl, *s;
	FILHDR *hdr;
	struct coff32 *coff;
	unsigned char buf[SYMESZ];

	coff  = obj->data;
	hdr = &coff->hdr;

	if (!coff->ents)
		return 1;

	strtbl = NULL;
	strsiz = 0;

	for (i = 0; i < hdr->f_nsyms; i++) {
		SYMENT *ent = &coff->ents[i];

		len = strlen(ent->n_name) + 1;
		if (len > strsiz - LONG_MAX)
			goto err;
		s = realloc(strtbl, strsiz + len);
		if (!s)
			goto err;
		memcpy(s + strsiz, ent->n_name, len);
		strtbl = s;
		strsiz += len;

		pack_ent(ORDER(obj->type), buf, ent);
		if (fwrite(buf, SYMESZ, 1, fp) != 1)
			return 0;
	}

	free(coff->strtbl);
	coff->strtbl = strtbl;
	coff->strsiz = strsiz;

	return 1;

err:
	free(strtbl);
	return 0;
}

static int
writestr(Obj *obj, FILE *fp)
{
	struct coff32 *coff;
	unsigned char buf[4];

	coff = obj->data;
	if ((coff->strsiz & 0xffff) != coff->strsiz)
		return 0;

	pack(ORDER(obj->type), buf, "l", coff->strsiz);
	fwrite(buf, 4, 1, fp);
	fwrite(coff->strtbl, coff->strsiz, 1, fp);

	return ferror(fp) == 0;
}

static int
writeaout(Obj *obj, FILE *fp)
{
	FILHDR *hdr;
	struct coff32 *coff;
	unsigned char buf[AOUTSZ];

	coff  = obj->data;
	hdr = &coff->hdr;

	if (hdr->f_opthdr == 0)
		return 1;
	pack_aout(ORDER(obj->type), buf, &coff->aout);

	return fread(buf, AOUTSZ, 1, fp) != 1;
}

static int
writereloc(Obj *obj, FILE *fp)
{
	int i, j;
	RELOC *rp;
	SCNHDR *scn;
	FILHDR *hdr;
	struct coff32 *coff;
	unsigned char buf[RELSZ];

	coff  = obj->data;
	hdr = &coff->hdr;

	if (!coff->rels)
		return 1;

	for (i = 0; i < hdr->f_nscns; i++) {
		rp = coff->rels[i];
		if (!rp)
			continue;
		scn = &coff->scns[i];

		for (j = 0; j < scn->s_nrelloc; j++) {
			pack_reloc(ORDER(obj->type), buf, &rp[i]);
			if (fwrite(buf, RELSZ, 1, fp) != 1)
				return 0;
		}
	}

	return 1;
}

static int
writelines(Obj *obj, FILE *fp)
{
	int i;
	long j;
	LINENO *lp;
	SCNHDR *scn;
	struct coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;
	unsigned char buf[LINESZ];

	if (!coff->lines)
		return 1;

        for (i = 0; i < hdr->f_nscns; i++) {
		lp = coff->lines[i];
		if (!lp)
			continue;
		scn = &coff->scns[i];
		for (j = 0; j < scn->s_nlnno; j++) {
			pack_line(ORDER(obj->type), buf, &lp[j]);
			if (fwrite(buf, LINESZ, 1, fp) == 1)
				return 0;
		}
	}

	return 1;
}

static int
writedata(Obj *obj, Map *map, FILE *fp)
{
	int id;
	long nsec;
	unsigned long long n;
	struct coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;
	SCNHDR *scn;
	Mapsec *sec;

	nsec = hdr->f_nscns;
	for (scn = coff->scns; nsec--; scn++) {
		if ((id = findsec(map, scn->s_name)) < 0)
			return 0;
		sec = &map->sec[id];
		fseek(sec->fp, sec->offset, SEEK_SET);

		for (n = sec->end - sec->begin; n > 0; n--)
			putc(getc(sec->fp), fp);
	}

	return !ferror(fp);
}

int
coff32write(Obj *obj, Map *map, FILE *fp)
{
	long ptr, n;
	SCNHDR *scn;
	struct coff32 *coff = obj->data;
	FILHDR *hdr = &coff->hdr;

	ptr = ftell(fp);
	obj->pos = ptr;
	ptr += FILHSZ + AOUTSZ + n*hdr->f_nscns;

	n = hdr->f_nscns;
	for (scn = coff->scns; n--; scn++) {
		scn->s_scnptr = ptr;
		ptr += scn->s_size;
	}

	n = hdr->f_nscns;
	for (scn = coff->scns; n--; scn++) {
		scn->s_relptr = ptr;
		ptr += scn->s_nrelloc * RELSZ;
	}

	n = hdr->f_nscns;
	for (scn = coff->scns; n--; scn++) {
		scn->s_lnnoptr = ptr;
		ptr += scn->s_nlnno * RELSZ;
	}

	/* and now update symbols */

	if (!writehdr(obj, fp))
		return -1;
	if (!writeaout(obj, fp))
		return -1;
	if (!writescns(obj, fp))
		return -1;
	if (!writedata(obj, map, fp))
		return -1;
	if (!writereloc(obj, fp))
		return -1;
	if (!writelines(obj, fp))
		return -1;
	if (!writeents(obj, fp))
		return -1;
	if (!writestr(obj, fp))
		return -1;

	return 0;
}
