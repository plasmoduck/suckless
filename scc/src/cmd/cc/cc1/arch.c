#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <scc/scc.h>
#include "cc1.h"

Type *voidtype, *pvoidtype, *booltype,
     *uchartype, *chartype, *schartype,
     *uinttype, *inttype,
     *sizettype, *pdifftype,
     *ushortype, *shortype,
     *longtype, *ulongtype,
     *ullongtype, *llongtype,
     *floattype, *doubletype, *ldoubletype,
     *ellipsistype, *va_list_type, *va_type;

Symbol *one, *zero;
char *architecture = "amd64-sysv";

static Arch *arch;

static Arch *
getarch(void)
{
	static struct archdef {
		char *arch;
		Arch *(*fun)(void);
	} *bp, defs[] = {
		"amd64-sysv", amd64_sysv,
		"arm64-sysv", arm64_sysv,
		"i386-sysv", i386_sysv,
		"z80-scc", z80_scc,
		NULL, NULL,
	};

	for (bp = defs; bp->arch; ++bp) {
		if (strcmp(bp->arch, architecture) == 0)
			return (*bp->fun)();
	}

	return NULL;
}

int
valid_va_list(Type *tp)
{
	return (*arch->valid_va_list)(tp);
}

void
iarch(void)
{
	if ((arch = getarch()) == NULL) {
		fprintf(stderr, "cc1: wrong architecture '%s'\n", architecture);
		exit(EXIT_FAILURE);
	}

	voidtype = &arch->voidtype;
	pvoidtype = &arch->pvoidtype;
	booltype = &arch->booltype;
	uchartype = &arch->uchartype;
	chartype = &arch->chartype;
	schartype = &arch->schartype;
	uinttype = &arch->uinttype;
	inttype = &arch->inttype;
	sizettype = &arch->sizettype;
	pdifftype = &arch->pdifftype;
	ushortype = &arch->ushortype;
	shortype = &arch->shortype;
	longtype = &arch->longtype;
	ulongtype = &arch->ulongtype;
	ullongtype = &arch->ullongtype;
	llongtype = &arch->llongtype;
	floattype = &arch->floattype;
	doubletype = &arch->doubletype;
	ldoubletype = &arch->ldoubletype;
	ellipsistype = &arch->ellipsistype;
	va_list_type = &arch->va_list_type;
	va_type = &arch->va_type;
	zero = &arch->zero;
	one = &arch->one;
}
