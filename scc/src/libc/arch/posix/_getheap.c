#include "../../libc.h"

void *
_getheap(void)
{
	extern char end[];

	return end;
}
