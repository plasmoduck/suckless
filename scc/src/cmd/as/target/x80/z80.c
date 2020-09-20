#include <stdlib.h>

#include <scc/scc.h>

#include "../../as.h"
#include "../x80/proc.h"

TUINT maxaddr = 0xFFFFFFFF;
int endian = LITTLE_ENDIAN;

void
iarch(void)
{
	static struct {
		char *name;
		char type;
	} regs[] = {
		"AF", AREG_AF,
		"A", AREG_A,
		"F", AREG_F,

		"BC", AREG_BC,
		"B", AREG_B,
		"C", AREG_C,

		"HL", AREG_HL,
		"H", AREG_H,
		"L", AREG_L,

		"DE", AREG_DE,
		"D", AREG_D,
		"E", AREG_E,

		"IX", AREG_IX,
		"IXL", AREG_IXL,
		"IXH", AREG_IXH,

		"IY", AREG_IY,
		"IYL", AREG_IYL,
		"IYH", AREG_IYH,

		"R", AREG_R,
		"I", AREG_I,
		"AF'", AREG_AF_,
		"SP", AREG_SP,

		"NZ", AREG_NZ,
		"Z", AREG_Z,
		"NC", AREG_NC,
		"PO", AREG_PO,
		"PE", AREG_PE,
		"P", AREG_P,
		"M", AREG_M,

		NULL,
	}, *bp;

	for (bp = regs; bp->name; ++bp) {
		Symbol *sym = lookup(bp->name);
		sym->flags = FREG;
		sym->value = bp->type;
	}
}
