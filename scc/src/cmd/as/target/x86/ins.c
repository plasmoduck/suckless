#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>

#include "../../as.h"
#include "proc.h"

#define addrbyte(mod, reg, rm) ((mod) << 6 | (reg) << 3 | (rm))

enum addr_mode {
	MEM_MODE   = 0,
	MEM8_MODE  = 1,
	MEM16_MODE = 2,
	REG_MODE   = 3,
};

static int
getclass(Node *np)
{
	if (np->addr != AREG)
		return 0;

	switch (np->sym->value) {
	case AREG_AL:
	case AREG_AH:
	case AREG_BL:
	case AREG_BH:
	case AREG_CL:
	case AREG_CH:
	case AREG_DL:
	case AREG_DH:
		return R8CLASS;

	case AREG_AX:
	case AREG_BX:
	case AREG_CX:
	case AREG_DX:
	case AREG_DI:
	case AREG_SI:
	case AREG_SP:
	case AREG_BP:
		return R16CLASS;

	case AREG_CS:
	case AREG_DS:
	case AREG_SS:
	case AREG_ES:
	case AREG_FS:
	case AREG_GS:

	case AREG_EFLAGS:
	case AREG_CF:
	case AREG_PF:
	case AREG_AF:
	case AREG_ZF:
	case AREG_SF:
	case AREG_TF:
	case AREG_IF:
	case AREG_DF:
	case AREG_OF:
	case AREG_IOPL:
	case AREG_NT:
	case AREG_RF:
	case AREG_VM:
	case AREG_AC:
	case AREG_VIF:
	case AREG_VIP:
	case AREG_ID:

	case AREG_EAX:
	case AREG_RAX:

	case AREG_EBX:
	case AREG_RBX:

	case AREG_ECX:
	case AREG_RCX:

	case AREG_EDX:
	case AREG_RDX:

	case AREG_SIL:
	case AREG_ESI:
	case AREG_RSI:
	case AREG_DIL:
	case AREG_EDI:
	case AREG_RDI:

	case AREG_SPL:
	case AREG_ESP:
	case AREG_RSP:

	case AREG_BPL:
	case AREG_EBP:
	case AREG_RBP:

	case AREG_R0:
	case AREG_MM0:
	case AREG_R1:
	case AREG_MM1:
	case AREG_R2:
	case AREG_MM2:
	case AREG_R3:
	case AREG_MM3:
	case AREG_R4:
	case AREG_MM4:
	case AREG_R5:
	case AREG_MM5:
	case AREG_R6:
	case AREG_MM6:
	case AREG_R7:
	case AREG_MM7:

	case AREG_R8:
	case AREG_R8L:
	case AREG_R8W:
	case AREG_R9:
	case AREG_R9L:
	case AREG_R9W:
	case AREG_R10:
	case AREG_R10L:
	case AREG_R10W:
	case AREG_R11:
	case AREG_R11L:
	case AREG_R11W:
	case AREG_R12:
	case AREG_R12L:
	case AREG_R12W:
	case AREG_R13:
	case AREG_R13L:
	case AREG_R13W:
	case AREG_R14:
	case AREG_R14L:
	case AREG_R14W:
	case AREG_R15:
	case AREG_R15L:
	case AREG_R15W:

	case AREG_XMM0:
	case AREG_XMM1:
	case AREG_XMM2:
	case AREG_XMM3:
	case AREG_XMM4:
	case AREG_XMM5:
	case AREG_XMM6:
	case AREG_XMM7:
	case AREG_XMM8:
	case AREG_XMM9:
	case AREG_XMM10:
	case AREG_XMM11:
	case AREG_XMM12:
	case AREG_XMM13:
	case AREG_XMM14:
	case AREG_XMM15:

	case AREG_YMM0:
	case AREG_YMM1:
	case AREG_YMM2:
	case AREG_YMM3:
	case AREG_YMM4:
	case AREG_YMM5:
	case AREG_YMM6:
	case AREG_YMM7:
	case AREG_YMM8:
	case AREG_YMM9:
	case AREG_YMM10:
	case AREG_YMM11:
	case AREG_YMM12:
	case AREG_YMM13:
	case AREG_YMM14:
	case AREG_YMM15:

	case AREG_MXCSR:
		return 0;
	default:
		abort();
	}
}

int
match(Op *op, Node **args)
{
	unsigned char *p;
	int arg, class, rep, opt;
	Node *np;

	if (!op->args)
		return args == NULL;

	opt = rep = 0;
	for (p = op->args; arg = *p; ++p) {
		if (rep)
			--p;
		if ((np = *args++) == NULL)
			return (rep|opt) != 0;

		switch (arg) {
		case AOPT:
			opt = 1;
			break;
		case AREP:
			rep = 1;
			break;
		case AREG_R8CLASS:
			class = R8CLASS;
			goto check_class;
		case AREG_R16CLASS:
			class = R16CLASS;
		check_class:
			if ((getclass(np) & class) == 0)
				return 0;
			break;
		case AIMM8:
		case AIMM16:
		case AIMM32:
		case AIMM64:
			if (np->addr != AIMM)
				return 0;
			if (toobig(np, arg))
				error("overflow in immediate operand");
			break;
		case ASYM:
			if (np->addr != AIMM || np->op != IDEN)
				return 0;
			break;
		case ADIRECT:
		case ASTR:
			if (np->addr != arg)
				return 0;
			break;
		default:
			abort();
		}
	}

	return *args == NULL;
}

Node *
moperand(void)
{
}

static int
reg8toint(Node *np)
{
	switch (np->sym->value) {
	case AREG_AL: return 0;
	case AREG_CL: return 1;
	case AREG_DL: return 2;
	case AREG_BL: return 3;
	case AREG_AH: return 4;
	case AREG_CH: return 5;
	case AREG_DH: return 6;
	case AREG_BH: return 7;
	default:      abort();
	}
}

static int
reg16toint(Node *np)
{
	switch (np->sym->value) {
	case AREG_AX: return 0;
	case AREG_CX: return 1;
	case AREG_DX: return 2;
	case AREG_BX: return 3;
	case AREG_SP: return 4;
	case AREG_BP: return 5;
	case AREG_SI: return 6;
	case AREG_DI: return 7;
	default:	abort();
	}
}

static int
reg32toint(Node *np)
{
	switch (np->sym->value) {
	case AREG_EAX: return 0;
	case AREG_ECX: return 1;
	case AREG_EDX: return 2;
	case AREG_EBX: return 3;
	case AREG_ESP: return 4;
	case AREG_EBP: return 5;
	case AREG_ESI: return 6;
	case AREG_EDI: return 7;
	default:	abort();
	}
}

void
reg8_reg8(Op *op, Node **args)
{
	int src, dst;
	char buf[op->size];

	src = reg8toint(args[0]);
	dst = reg8toint(args[1]);
	memcpy(buf, op->bytes, op->size - 1);
	buf[op->size - 1] = addrbyte(REG_MODE, src, dst);
	emit(buf, op->size);
}

void
imm8_reg8(Op *op, Node **args)
{
	int src, dst;
	char buf[op->size];

	src = (*args)->sym->value;
	dst = reg8toint(args[1]);
	memcpy(buf, op->bytes, op->size - 2);
	buf[op->size - 2] = addrbyte(REG_MODE, 0, dst);
	buf[op->size - 1] = src;
	emit(buf, op->size);
}


void
reg16_reg16(Op *op, Node **args)
{
	int src, dst;
	char buf[op->size];

	src = reg16toint(args[0]);
	dst = reg16toint(args[1]);
	memcpy(buf, op->bytes, op->size - 1);
	buf[op->size - 1] = addrbyte(REG_MODE, src, dst);
	emit(buf, op->size);
}


void
reg32_reg32(Op *op, Node **args)
{
	int src, dst;
	char buf[op->size];

	src = reg32toint(args[0]);
	dst = reg32toint(args[1]);
	memcpy(buf, op->bytes, op->size - 1);
	buf[op->size - 1] = addrbyte(REG_MODE, src, dst);
	emit(buf, op->size);
}
