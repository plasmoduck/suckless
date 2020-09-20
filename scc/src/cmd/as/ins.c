#include <string.h>

#include <scc/scc.h>
#include "as.h"

extern Section *sabs, *sbss, *sdata, *stext;

enum {
	EQU,
	COMMON,
	SIZE,
	XSTRING,
	ASCII,
	TYPE,
};

static void
reloc(Symbol *sym,
       unsigned flags,
       unsigned size,
       unsigned nbits,
       unsigned shift)
{
}

char *
tobytes(TUINT v, int nbytes, int inc)
{
	static char buf[sizeof(TUINT)];
	int idx;

	idx = (inc < 0) ? nbytes-1 : 0;
	while (nbytes--) {
		buf[idx] = v;
		idx += inc;
		v >>= 8;
	}

	if (v)
		error("overflow in immediate value");
	return buf;
}

void
noargs(Op *op, Node **args)
{
	emit(op->bytes, op->size);
}

static void
xstring(int which, Node **args)
{
	Node *np;
	char *s;
	size_t len;

	while (np = *args++) {
		s = np->sym->name.buf;
		len = strlen(s);
		len += which == XSTRING;
		emit(s, len);
	}
}

void
string(Op *op, Node **args)
{
	xstring(STRING, args);
}

void
ascii(Op *op, Node **args)
{
	xstring(STRING, args);
}

void
def(Node **args, int siz)
{
	Node *np;

	while (np = *args++) {
		Symbol *sym = np->sym;

		if ((sym->flags & FABS) == 0)
			reloc(sym, 0, siz, siz * 8, 0);
		emit(tobytes(sym->value, siz, endian), siz);
	}
}

void
defb(Op *op, Node **args)
{
	def(args, 1);
}

void
defw(Op *op, Node **args)
{
	def(args, 2);
}

void
defd(Op *op, Node **args)
{
	def(args, 4);
}

void
defq(Op *op, Node **args)
{
	def(args, 8);
}

static void
symexp(int which, Op *op, Node **args)
{
	Symbol *sym, *exp;
	static char *cmds[] = {
		[EQU] = "equ",
		[COMMON] = "common",
		[SIZE] = "size",
	};
	char *cmd = cmds[which];

	if (args[1]) {
		sym = args[0]->sym;
		exp = args[1]->sym;
	} else if (linesym) {
		sym = linesym;
		exp = args[0]->sym;
	} else {
		error("%s pseudo instruction lacks a label", cmd);
	}

	if ((exp->flags & FABS) == 0)
		error("%s expression is not an absolute expression", cmd);

	switch (which) {
	case EQU:
		if (pass == 1 && (sym->flags & FDEF))
			error("redefinition of symbol '%s'", sym->name.buf);
		sym->value = exp->value;
		sym->flags |= FDEF;
		break;
	case COMMON:
		sym->flags |= FCOMMON;
	case SIZE:
		sym->size = exp->value;
		break;
	case TYPE:
		sym->type.buf = xstrdup(exp->name.buf);
		break;
	}
}

void
equ(Op *op, Node **args)
{
	symexp(EQU, op, args);
}

void
common(Op *op, Node **args)
{
	symexp(COMMON, op, args);
}

void
size(Op *op, Node **args)
{
	symexp(SIZE, op, args);
}

void
type(Op *op, Node **args)
{
	symexp(TYPE, op, args);
}

void
section(Op *op, Node **args)
{
	Symbol *sym = args[0]->sym;
	char *attr = NULL;

	if (args[1])
		attr = args[1]->sym->name.buf;

	setsec(sym->name.buf, attr);
}

void
text(Op *op, Node **args)
{
	cursec = stext;
}

void
data(Op *op, Node **args)
{
	cursec = sdata;
}

void
bss(Op *op, Node **args)
{
	cursec = sbss;
}

void
extrn(Op *op, Node **args)
{
	Symbol *sym = args[0]->sym;

	sym->flags |= FEXTERN;
}

void
global(Op *op, Node **args)
{
	Symbol *sym = args[0]->sym;

	sym->flags |= FGLOBAL;
}

void
align(Op *op, Node **args)
{
	Symbol *sym = args[0]->sym;
	TUINT curpc, pc, al;

	if ((sym->flags & FABS) == 0)
		error("align expression is not an absolute expression");
	if ((al = sym->value) == 0)
		return;

	al--;
	curpc = cursec->curpc;
	pc = curpc+al & ~al;

	for (al = pc - curpc; al > 0; --al)
		emit((char []) {0}, 1);
}

void
end(Op *op, Node **args)
{
	endpass = 1;
}

void
include(Op *op, Node **args)
{
	addinput(args[0]->sym->name.buf);
}
