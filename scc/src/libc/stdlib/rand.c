#include <stdlib.h>
#undef rand
#undef srand

static unsigned long next;

void
srand(unsigned seed)
{
	next = seed;
}

int
rand(void)  /* RAND_MAX assumed to be 32767. */
{
	next = next * 1103515245 + 12345;
	return (unsigned)(next/65536) % 32768;
}
