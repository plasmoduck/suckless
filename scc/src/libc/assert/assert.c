#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#undef assert

void
assert(int exp)
{
	if (exp)
		return;
	fputs("assert failed\n", stderr);
	abort();
}
