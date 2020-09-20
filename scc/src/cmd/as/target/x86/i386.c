#include <scc/scc.h>

#include "../../as.h"
#include "../x86/proc.h"

TUINT maxaddr = 0xFFFFFFFF;
int endian = LITTLE_ENDIAN;

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
		"FS", AREG_FS,
		"GS", AREG_GS,

		"AX", AREG_AX,
		"AL", AREG_AL,
		"AH", AREG_AH,
		"EAX", AREG_EAX,

		"BC", AREG_BX,
		"BL", AREG_BL,
		"BH", AREG_BH,
		"EBX", AREG_EBX,

		"CX", AREG_CX,
		"CL", AREG_CL,
		"CH", AREG_CH,
		"ECX", AREG_ECX,

		"DX", AREG_DX,
		"DL", AREG_DL,
		"DH", AREG_DH,
		"EDX", AREG_EDX,

		"SI", AREG_SI,
		"ESI", AREG_ESI,
		"DI", AREG_DI,
		"EDI", AREG_EDI,

		"SP", AREG_SP,
		"ESP", AREG_ESP,

		"BP", AREG_BP,
		"EBP", AREG_EBP,

		"R0", AREG_R0,
		"MM0", AREG_MM0,
		"R1", AREG_R1,
		"MM1", AREG_MM1,
		"R2", AREG_R2,
		"MM2", AREG_MM2,
		"R3", AREG_R3,
		"MM3", AREG_MM3,
		"R4", AREG_R4,
		"MM4", AREG_MM4,
		"R5", AREG_R5,
		"MM5", AREG_MM5,
		"R6", AREG_R6,
		"MM6", AREG_MM6,
		"R7", AREG_R7,
		"MM7", AREG_MM7,

		"XMM0", AREG_XMM0,
		"XMM1", AREG_XMM1,
		"XMM2", AREG_XMM2,
		"XMM3", AREG_XMM3,
		"XMM4", AREG_XMM4,
		"XMM5", AREG_XMM5,
		"XMM6", AREG_XMM6,
		"XMM7", AREG_XMM7,

		"YMM0", AREG_YMM0,
		"YMM1", AREG_YMM1,
		"YMM2", AREG_YMM2,
		"YMM3", AREG_YMM3,
		"YMM4", AREG_YMM4,
		"YMM5", AREG_YMM5,
		"YMM6", AREG_YMM6,
		"YMM7", AREG_YMM7,

		"MXCSR", AREG_MXCSR,

		NULL
	}, *bp;

	for (bp = regs; bp->name; ++bp) {
		Symbol *sym = lookup(bp->name);
		sym->flags = FREG;
		sym->value = bp->type;
	}
}
