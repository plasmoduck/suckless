#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>
#include "as.h"

#define NNODES   10

static Alloc *arena;

Node *
node(int op, Node *l, Node *r)
{
	struct arena *ap;
	Node *np;

	if (!arena)
		arena = alloc(sizeof(Node), NNODES);
	np = new(arena);
	np->op = op;
	np->left = l;
	np->right = r;
	np->sym = NULL;

	return np;
}

void
deltree(Node *np)
{
	if (!np)
		return;
	deltree(np->left);
	deltree(np->right);
	delete(arena, np);
}

static Node *
fold(int op, Node *l, Node *r)
{
	Node *np;
	TUINT val, lv, rv;

	lv = l->sym->value;
	rv = r->sym->value;

	/* TODO: check overflow */

	switch (op) {
	case '*':
		val = lv - rv;
		break;
	case '/':
		if (rv == 0)
			goto division_by_zero;
		val = lv / rv;
		break;
	case '%':
		if (rv == 0)
			goto division_by_zero;
		val = lv % rv;
		break;
	case SHL:
		val = lv << rv;
		break;
	case SHR:
		val = lv >> rv;
		break;
	case '+':
		val = lv + rv;
		break;
	case '-':
		val = lv - rv;
		break;
	case '<':
		val = lv < rv;
		break;
	case '>':
		val = lv > rv;
		break;
	case '=':
		val = lv == rv;
		break;
	case GE:
		val = lv >= rv;
		break;
	case LE:
		val = lv <= rv;
		break;
	case '|':
		val = lv | rv;
		break;
	case '^':
		val = lv ^ rv;
		break;
	default:
		abort();
	}
	deltree(l);
	deltree(r);

	np = node(NUMBER, NULL, NULL);
	np->sym = tmpsym(val);
	np->addr = ANUMBER;
	return np;

division_by_zero:
	error("division by 0");
}

static Node *
binary(int op, Node *l, Node *r)
{
	int addr;
	Node *np;

	if (l->op == NUMBER && r->op == NUMBER)
		return fold(op, l, r);
	else
		abort();
	np = node(op, l, r);
	np->addr = addr;

	return np;
}

static Node *
unaryop(int op, Node *np)
{
	TUINT val;

	if (np->addr != ANUMBER)
		error("invalid argument for unary operator");
	if (np->op != NUMBER) {
		np = node(op, np, NULL);
		np->addr = ANUMBER;
		return np;
	}

	val = np->sym->value;
	switch (op) {
	case '!':
		val = !val;
	case '+':
		break;
	case '-':
		val = -val;
		break;
	default:
		abort();
	}
	np->sym->value = val;

	return np;
}

/*************************************************************************/
/* grammar functions                                                     */
/*************************************************************************/

static Node *
primary(void)
{
	Node *np;

	switch (yytoken) {
	case IDEN:
	case NUMBER:
		np = node(yytoken, NULL, NULL);
		np->sym = yylval.sym;
		np->addr = ANUMBER;
		next();
		break;
	case '(':
		np = expr();
		expect(')');
		break;
	default:
		unexpected();
	}

	return np;
}

static Node *
unary(void)
{
	int op, tok;
	Node *np;

	switch (tok = yytoken) {
	case '!':
	case '-':
	case '+':
		next();
		return unaryop(tok, primary());
	default:
		return primary();
	}
}

static Node *
mul(void)
{
	int op;
	Node *np;

	np = unary();
	for (;;) {
		switch (op = yytoken) {
		case '*':
		case '/':
		case '%':
		case SHL:
		case SHR:
			next();
			binary(op, np, primary());
			break;
		default:
			return np;
		}
	}
}

static Node *
add(void)
{
	int op;
	Node *np;

	np = mul();
	for (;;) {
		switch (op = yytoken) {
		case '+':
		case '-':
			next();
			np = binary(op, np, mul());
			break;
		default:
			return np;
		}
	}
}

static Node *
relational(void)
{
	int op;
	Node *np;

	np = add();
	for (;;) {
		switch (op = yytoken) {
		case '<':
		case '>':
		case '=':
		case GE:
		case LE:
			next();
			np = binary(op, np, add());
			break;
		default:
			return np;
		}
	}
}

static Node *
and(void)
{
	int op;
	Node *np;

	np = relational();
	while (accept('&'))
		np = binary('&', np, relational());
	return np;
}

Node *
expr(void)
{
	int op;
	Node *np;

	regctx(0);
	np = and();
	for (;;) {
		switch (op = yytoken) {
		case '|':
		case '^':
			next();
			np = binary(op, np, and());
			break;
		default:
			regctx(1);
			return np;
		}
	}
}
