#include <stdlib.h>

#include <scc/scc.h>

#include "../../as.h"
#include "proc.h"

TUINT maxaddr = 0xFFFF;
int endian = BIG_ENDIAN;
int left2right = 0;

void
iarch(void)
{
	static struct {
		char *name;
		char type;
	} regs[] = {
		"R0", AREG_R0,
		"R1", AREG_R1,
		"R2", AREG_R2,
		"R3", AREG_R3,
		"R4", AREG_R4,
		"R5", AREG_R5,
		"R6", AREG_R6,
		"R7", AREG_R7,
		"R8", AREG_R8,
		"R9", AREG_R9,

		"R10", AREG_R10,
		"R11", AREG_R11,
		"R12", AREG_R12,
		"R13", AREG_R13,
		"R14", AREG_R14,
		"R15", AREG_R15,
		"R16", AREG_R16,
		"R17", AREG_R17,
		"R18", AREG_R18,
		"R19", AREG_R19,

		"R20", AREG_R20,
		"R21", AREG_R21,
		"R22", AREG_R22,
		"R23", AREG_R23,
		"R24", AREG_R24,
		"R25", AREG_R25,
		"R26", AREG_R26,
		"R27", AREG_R27,
		"R28", AREG_R28,
		"R29", AREG_R29,
		"R30", AREG_R30,
		"R31", AREG_R31,

		NULL
	}, *bp;

	for (bp = regs; bp->name; ++bp) {
		Symbol *sym = lookup(bp->name);
		sym->flags = FREG;
		sym->value = bp->type;
	}
}
