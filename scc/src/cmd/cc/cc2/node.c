#include <stdlib.h>
#include <string.h>

#include <scc/scc.h>

#include "cc2.h"

#define NNODES   32

Node *curstmt;
Symbol *curfun;

static Alloc *arena;


Node *
node(int op)
{
	struct arena *ap;
	Node *np;

	if (!arena)
		arena = alloc(sizeof(Node), NNODES);
	np = memset(new(arena), 0, sizeof(*np));
	np->op = op;

	return np;
}

#ifndef NDEBUG
#include <stdio.h>

static void
prnode(Node *np)
{
	if (np->left)
		prnode(np->left);
	if (np->right)
		prnode(np->right);
	fprintf(stderr, "\t%c%lu", np->op, np->type.size);
}

void
prtree(Node *np)
{
	prnode(np);
	putc('\n', stderr);
}

void
prforest(char *msg)
{
	Node *np;

	if (!curfun)
		return;

	fprintf(stderr, "%s {\n", msg);
	for (np = curfun->u.stmt; np; np = np->next)
		prtree(np);
	fputs("}\n", stderr);
}
#endif

Node *
addstmt(Node *np, int flag)
{
	if (curstmt)
		np->next = curstmt->next;
	np->prev = curstmt;

	if (!curfun->u.stmt)
		curfun->u.stmt = np;
	else
		curstmt->next = np;

	if (flag == SETCUR)
		curstmt = np;

	return np;
}

Node *
delstmt(void)
{
	Node *next, *prev;

	next = curstmt->next;
	prev = curstmt->prev;
	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;
	else
		curfun->u.stmt = next;
	deltree(curstmt);

	return curstmt = next;
}

Node *
nextstmt(void)
{
	return curstmt = curstmt->next;
}

void
delnode(Node *np)
{
	delete(arena, np);
}

void
deltree(Node *np)
{
	if (!np)
		return;
	deltree(np->left);
	deltree(np->right);
	delnode(np);
}

void
cleannodes(void)
{
	if (arena) {
		dealloc(arena);
		arena = NULL;
	}
	curstmt = NULL;
}

void
apply(Node *(*fun)(Node *))
{
	if (!curfun)
		return;
	curstmt = curfun->u.stmt;
	while (curstmt)
		(*fun)(curstmt) ? nextstmt() : delstmt();
}
