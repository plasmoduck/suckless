#include <stdlib.h>

#include <scc/scc.h>

#include "../../as.h"
#include "proc.h"

/*
 * This code is derived from PowerISA_V2.06B_V2_PUBLIC document.
 * All the names used in the specification are preserved in
 * this code.
 */

static int
getclass(Node *np)
{
	if (np->addr != AREG)
		return 0;

	switch (np->sym->value) {
	case AREG_R0:
	case AREG_R1:
	case AREG_R2:
	case AREG_R3:
	case AREG_R4:
	case AREG_R5:
	case AREG_R6:
	case AREG_R7:
	case AREG_R8:
	case AREG_R9:
	case AREG_R10:
	case AREG_R11:
	case AREG_R12:
	case AREG_R13:
	case AREG_R14:
	case AREG_R15:
	case AREG_R16:
	case AREG_R17:
	case AREG_R18:
	case AREG_R19:
	case AREG_R20:
	case AREG_R21:
	case AREG_R22:
	case AREG_R23:
	case AREG_R24:
	case AREG_R25:
	case AREG_R26:
	case AREG_R27:
	case AREG_R29:
	case AREG_R30:
	case AREG_R31:
		return GPRSCLASS;
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
		case AREG_GPRSCLASS:
			class = GPRSCLASS;
		check_class:
			if ((getclass(np) & class) == 0)
				return 0;
			break;
		case AIMM2:
		case AIMM5:
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
	abort();
}

static void
emit_packed(unsigned long ins)
{
	char buff[4];

	if (endian == BIG_ENDIAN) {
		buff[0] = ins >> 24;
		buff[1] = ins >> 16;
		buff[2] = ins >> 8;
		buff[3] = ins;
	} else {
		buff[0] = ins;
		buff[1] = ins >> 8;
		buff[2] = ins >> 16;
		buff[3] = ins >> 24;
	}

	emit(buff, 4);
}

void
i_form(Op *op, Node **args)
{
	unsigned long ins, opcd, li, aa, lk;
	long long dst;
	long long max = 1l << 23;
	long long min = -(1l << 23);

	opcd = op->bytes[0];
	aa = op->bytes[1];
	lk = op->bytes[2];

	dst = args[0]->sym->value;
	if (dst & 0x3)
		error("unaligned branch");
	if (aa)
		dst -= cursec->curpc - 4;
	if (dst < min || dst > max)
		error("out of range branch");

	li = dst;
	li >>= 2;
	ins = opcd<<26 | li<<2 | aa<<1 | lk;
	emit_packed(ins);
}

void
b_form(Op *op, Node **args)
{
	unsigned long ins, opcd, bo, bi, bd, aa, lk;
	long long dst;
	long long max = 1l << 13;
	long long min = -(1l << 13);

	opcd = op->bytes[0];
	aa = op->bytes[1];
	lk = op->bytes[2];

	bo = args[0]->sym->value;
	bi = args[1]->sym->value;

	dst = args[2]->sym->value;
	if (dst & 0x3)
		error("unaligned branch");
	if (aa)
		dst -= cursec->curpc - 4;

	if (dst < min || dst > max)
		error("out of range branch");
	bd = dst;
	bd >>= 2;

	ins = opcd<<26 | bo<<21 | bi<<16 | bd<<11 | aa<<1 | lk;
	emit_packed(ins);
}

void
sc_form(Op *op, Node **args)
{
	abort();
}

void
d_form(Op *op, Node **args)
{
	abort();
}

void
ds_form(Op *op, Node **args)
{
	abort();
}

void
dq_form(Op *op, Node **args)
{
	abort();
}

void
x_form(Op *op, Node **args)
{
	abort();
}

void
xl_form(Op *op, Node **args)
{
	unsigned long ins, bo, bi, bh, lk;
	unsigned long opcd1, opcd2;
	long long dst;

	opcd1 = op->bytes[0];
	opcd2 = op->bytes[1]<<8 | op->bytes[2];
	lk = op->bytes[3];

	bo = args[0]->sym->value;
	bi = args[1]->sym->value;
	bh = args[2]->sym->value;

	ins = opcd1<<26 | bo<<21 | bi<<16 | bh<<11 | opcd2<<1 | lk;
	emit_packed(ins);
}

void
xfx_form(Op *op, Node **args)
{
	abort();
}

void
xlfdorm_form(Op *op, Node **args)
{
	abort();
}

void
xx1_form(Op *op, Node **args)
{
	abort();
}

void
xx2_form(Op *op, Node **args)
{
	abort();
}

void
xx3_form(Op *op, Node **args)
{
	abort();
}

void
xx4_form(Op *op, Node **args)
{
	abort();
}

void
xs_form(Op *op, Node **args)
{
	abort();
}

void
xo_form(Op *op, Node **args)
{
	abort();
}

void
a_form(Op *op, Node **args)
{
	abort();
}

void
m_form(Op *op, Node **args)
{
	abort();
}

void
md_form(Op *op, Node **args)
{
	abort();
}

void
mds_form(Op *op, Node **args)
{
	abort();
}

void
va_form(Op *op, Node **args)
{
	abort();
}

void
vc_form(Op *op, Node **args)
{
	abort();
}

void
vx_form(Op *op, Node **args)
{
	abort();
}

void
evs_form(Op *op, Node **args)
{
	abort();
}

void
z22_form(Op *op, Node **args)
{
	abort();
}

void
z23_form(Op *op, Node **args)
{
	abort();
}
