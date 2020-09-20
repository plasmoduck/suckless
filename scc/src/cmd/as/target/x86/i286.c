#include <scc/scc.h>

#include "../../as.h"
#include "../x86/proc.h"

TUINT maxaddr = 0xFFFF;
int endian = LITTLE_ENDIAN;
int left2right = 0;

void
iarch(void)
{
	static struct {
		char *name;
		char type;
	} regs[] = {
		"CS", AREG_CS,
		"DS", AREG_DS,
		"SS", AREG_SS,
		"ES", AREG_ES,

		"AX", AREG_AX,
		"AL", AREG_AL,
		"AH", AREG_AH,

		"BX", AREG_BX,
		"BL", AREG_BL,
		"BH", AREG_BH,

		"CX", AREG_CX,
		"CL", AREG_CL,
		"CH", AREG_CH,

		"DX", AREG_DX,
		"DL", AREG_DL,
		"DH", AREG_DH,

		"SI", AREG_SI,
		"DI", AREG_DI,

		"SP", AREG_SP,
		"BP", AREG_BP,

		NULL
	}, *bp;

	for (bp = regs; bp->name; ++bp) {
		Symbol *sym = lookup(bp->name);
		sym->flags = FREG;
		sym->value = bp->type;
	}
}
