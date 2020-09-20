#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>

#include "../../as.h"
#include "proc.h"

/*
 * This implementation is based in:
 *	- Zilog Z80 CPU Specifications by Sean Young
 *	- Decoding Z80 opcodes - of use to disassembler and emulator
 *	  writers - by Cristian Dinu.
 */

static int
getclass(Node *np)
{
	if (np->addr != AREG)
		return 0;

	switch (np->sym->value) {
	case AREG_C:
		return RCLASS | PCLASS | QCLASS | CCCLASS | SSCLASS;
	case AREG_A:
	case AREG_B:
	case AREG_D:
	case AREG_E:
		return RCLASS | PCLASS | QCLASS;
	case AREG_H:
	case AREG_L:
		return RCLASS;
	case AREG_IXL:
	case AREG_IXH:
		return PCLASS;
	case AREG_IYL:
	case AREG_IYH:
		return QCLASS;
	case AREG_HL:
		return DDCLASS | QQCLASS;
	case AREG_BC:
	case AREG_DE:
		return DDCLASS | QQCLASS | PPCLASS | RRCLASS;
	case AREG_SP:
		return DDCLASS | PPCLASS | RRCLASS;
	case AREG_AF:
		return QQCLASS;
	case AREG_IX:
		return PPCLASS;
	case AREG_IY:
		return RRCLASS;
	case AREG_PO:
	case AREG_PE:
	case AREG_P:
	case AREG_M:
		return CCCLASS;
	case AREG_NZ:
	case AREG_Z:
	case AREG_NC:
		return CCCLASS | SSCLASS;
	default:
		return 0;
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
		case AINDER_C:
			arg = AREG_C;
			goto indirect;
		case AINDER_HL:
			arg = AREG_HL;
			goto indirect;
		case AINDER_DE:
			arg = AREG_DE;
			goto indirect;
		case AINDER_BC:
			arg = AREG_BC;
			goto indirect;
		case AINDER_IX:
			arg = AREG_IX;
			goto indirect;
		case AINDER_IY:
			arg = AREG_IY;
			goto indirect;
		case AINDER_SP:
			arg = AREG_SP;
		indirect:
			if (np->addr != AINDIR)
				return 0;
			np = np->left;
		case AREG_A:
		case AREG_I:
		case AREG_R:
		case AREG_F:
		case AREG_HL:
		case AREG_BC:
		case AREG_DE:
		case AREG_IY:
		case AREG_IX:
		case AREG_SP:
		case AREG_AF:
		case AREG_AF_:
			if (np->addr != AREG || np->sym->value != arg)
				return 0;
			break;
		case AREG_RCLASS:
			class = RCLASS;
			goto check_class;
		case AREG_PCLASS:
			class = PCLASS;
			goto check_class;
		case AREG_QCLASS:
			class = QCLASS;
			goto check_class;
		case AREG_QQCLASS:
			class = QQCLASS;
			goto check_class;
		case AREG_PPCLASS:
			class = PPCLASS;
			goto check_class;
		case AREG_RRCLASS:
			class = RRCLASS;
			goto check_class;
		case AREG_CCCLASS:
			class = CCCLASS;
			goto check_class;
		case AREG_SSCLASS:
			class = SSCLASS;
			goto check_class;
		case AREG_DDCLASS:
			class = DDCLASS;
		check_class:
			if ((getclass(np) & class) == 0)
				return 0;
			break;
		case AINDEX_IY:
			arg = AREG_IY;
			goto index_address;
		case AINDEX_IX:
			arg = AREG_IX;
		index_address:
			if (np->addr != AINDEX)
				return 0;
			np = np->left->left;
			if (np->sym->value != arg)
				return 0;
			if (toobig(np, AIMM8))
				error("overflow in index");
			break;
		case ARST:
			if (np->addr != AIMM)
				return 0;
			if ((np->sym->value & ~0x38) != 0)
				return 0;
			break;
		case AZERO:
			if (np->addr != AIMM)
				return 0;
			break;
		case AIMM3:
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

/*
 * (expr) -> ADIRECT
 * (REG)  -> AINDIR
 * (REG + expr) -> AINDEX
 * (REG - expr) -> AINDEX
 * expr (REG) -> AINDEX
 */
Node *
moperand(void)
{
	int op;
	Node *np, *dir, *off, *reg;

	dir = off = reg = NULL;
	if (accept('(')) {
		if (yytoken != REG) {
			dir = expr();
		} else {
			reg = getreg();
			switch (yytoken) {
			case '+':
			case '-':
				off = expr();
			case ')':
				break;
			default:
				unexpected();
			}
		}
	} else {
		off = expr();
		expect('(');
		reg = getreg();
	}
	expect(')');

	if (dir) {
		op = ADIRECT;
		np = dir;
	} else if (off) {
		np = node(AREG_OFF, reg, off);
		op = AINDEX;
	} else {
		np = reg;
		op = AINDIR;
	}
	np = node(op, np, NULL);
	np->addr = op;
	return np;
}

static int
reg2int(Node *np)
{
	switch (np->sym->value) {
	case AREG_F:
	case AREG_B:   return 0;
	case AREG_C:   return 1;
	case AREG_D:   return 2;
	case AREG_E:   return 3;
	case AREG_IXH:
	case AREG_IYH:
	case AREG_H:   return 4;
	case AREG_IXL:
	case AREG_IYL:
	case AREG_L:   return 5;
	case AREG_A:   return 7;
	case AREG_BC:  return 0;
	case AREG_DE:  return 1;
	case AREG_HL:
	case AREG_IX:
	case AREG_IY:  return 2;
	case AREG_AF:
	case AREG_SP:  return 3;
	default:       abort();
	}
}

static int
cc2int(Node *np)
{
	switch (np->sym->value) {
	case AREG_NZ:  return 0;
	case AREG_Z:   return 1;
	case AREG_NC:  return 2;
	case AREG_C:   return 3;
	case AREG_PO:  return 4;
	case AREG_PE:  return 5;
	case AREG_P:   return 6;
	case AREG_M:   return 7;
	default:       abort();
	}
}

static int
ss2int(Node *np)
{
	switch (np->sym->value) {
	case AREG_NZ:  return 4;
	case AREG_Z:   return 5;
	case AREG_NC:  return 6;
	case AREG_C:   return 7;
	default:       abort();
	}
}

void
dir(Op *op, Node **args)
{
	Node *imm;
	unsigned char buf[4];
	unsigned val;
	int n = op->size;

	imm = (args[1]->addr == ADIRECT) ? args[1] : args[0];
	imm = imm->left;
	memcpy(buf, op->bytes, n);
	val = imm->sym->value;
	buf[n-1] = val >> 8;
	buf[n-2] = val;
	emit(buf, n);
}

void
ld8(Op *op, Node **args)
{
	Node *par1 = args[0], *par2 = args[1];
	int n = op->size, i = n;;
	unsigned regval = 0;
	unsigned char buf[4];

	memcpy(buf, op->bytes, n);

	if (par1->addr == AREG)
		regval |= reg2int(par1) << 3;
	if (par2->addr == AREG)
		regval |= reg2int(par2);
	else if (par2->addr == AIMM)
		buf[--i] = par2->sym->value;

	buf[--i] |= regval;
	emit(buf, n);
}

void
alu16(Op *op, Node **args)
{
	Node *par;
	int n = op->size;
	unsigned val;
	unsigned char buf[4];

	par = (args[1]) ? args[1] : args[0];
	val = reg2int(par);
	memcpy(buf, op->bytes, n);
	buf[n-1] |= val << 4;
	emit(buf, n);
}

void
ld16(Op *op, Node **args)
{
	Node *dst, *src, *tmp;
	int n = op->size;
	unsigned val;
	unsigned char buf[4];

	dst = args[0];
	src = args[1];
	if (!src) {
		alu16(op, args);
		return;
	}

	if (dst->addr != AREG) {
		tmp = src;
		src = dst;
		dst = tmp;
	}

	memcpy(buf, op->bytes, n);
	if (src->addr == ADIRECT)
		src = src->left;
	val = src->sym->value;
	buf[n-1] = val >> 8;
	buf[n-2] = val;
	buf[n-3] |= reg2int(dst) << 4;
	emit(buf, n);
}

void
alu8(Op *op, Node **args)
{
	Node *par = args[1];
	unsigned char buf[4];
	int n = op->size, shift;
	unsigned val;

	if (args[1]) {
		shift = 0;
		par = args[1];
	} else {
		shift = 3;
		par = args[0];
	}

	switch (par->addr) {
	case AIMM:
		val = par->sym->value;
		break;
	case AREG:
		val = reg2int(par) << shift;
		break;
	case AINDEX:
		val = par->left->right->sym->value;
		break;
	case AINDIR:
		val = 0;
		break;
	default:
		abort();
	}

	memcpy(buf, op->bytes, n);
	buf[n-1] |= val;
	emit(buf, n);
}

void
idx(Op *op, Node **args)
{
	Node *tmp, *idx, *imm, *reg;
	unsigned char buf[4];
	int n = op->size, i = n, shift = 0;

	imm = reg = NULL;
	if (args[0]->addr != AINDEX) {
		shift = 3;
		tmp = args[0];
		args[0] = args[1];
		args[1] = tmp;
	}
	idx = args[0]->left->right;

	if (args[1]->addr == AREG)
		reg = args[1];
	else
		imm = args[1];

	memcpy(buf, op->bytes, n);

	if (imm)
		buf[--i] = imm->sym->value;
	buf[--i] = idx->sym->value;
	if (reg)
		buf[--i] |= reg2int(reg) << shift;

	emit(buf, n);
}

void
inout(Op *op, Node **args)
{
	Node *port, *value;
	unsigned val;
	int n = op->size;
	unsigned char buf[5];

	port = args[0];
	value = args[1];
	if (port->addr != ADIRECT && port->addr != AINDIR) {
		value = port;
		port = args[1];
	}

	if (port->addr == ADIRECT)
		val = port->left->sym->value;
	else if (value->addr == AREG)
		val = reg2int(value) << 3;
	else
		val = 0;

	memcpy(buf, op->bytes, n);
	buf[n-1] |= val;
	emit(buf, n);
}

void
rot_bit(Op *op, Node **args)
{
	Node *par = args[0];
	unsigned char buf[5];
	int n = op->size;
	unsigned val, npar = 0;

	memcpy(buf, op->bytes, n);

	par = args[0];
	if (par->addr == AIMM) {
		buf[n-1] |= par->sym->value << 3;
		par = args[npar = 1];
	}

	switch (par->addr) {
	case AINDEX:
		val = par->left->right->sym->value;
		buf[n-2] = val;
		par = args[npar+1];
		if (!par)
			break;
	case AREG:
		val = reg2int(par);
		buf[n-1] |= val;
	case AINDIR:
		break;
	default:
		abort();
	}

	emit(buf, n);
}

void
im(Op *op, Node **args)
{
	unsigned val = args[0]->sym->value;
	unsigned char buf[4];
	int n = op->size;

	if (val > 0)
		++val;

	memcpy(buf, op->bytes, n);
	buf[n-1] |= val << 3;
	emit(buf, n);
}

void
branch(int relative, Op *op, Node **args)
{
	unsigned char buf[4];
	Node *flag, *imm;
	int n = op->size, i = n;
	unsigned val;
	int (*fun)(Node *);

	flag = imm = NULL;
	if (args[0]->addr == AREG) {
		flag = args[0];
		imm = args[1];
	} else if (args[0]->addr == AIMM) {
		imm = args[0];
	}
	memcpy(buf, op->bytes, n);

	if (imm) {
		val = imm->sym->value;
		if (!relative)
			buf[--i] = val >> 8;
		else
			val -= cursec->curpc - 2;
		buf[--i] = val;

	}
	if (flag) {
		fun = (relative) ? ss2int : cc2int;
		buf[--i] |= (*fun)(flag) << 3;
	}


	emit(buf, n);
}

void
jp(Op *op, Node **args)
{
	branch(0, op, args);
}

void
jr(Op *op, Node **args)
{
	branch(1, op, args);
}

void
rst(Op *op, Node **args)
{
	unsigned char buf[1];

	buf[0] = op->bytes[0];
	buf[0] |= args[0]->sym->value;
	emit(buf, 1);
}
