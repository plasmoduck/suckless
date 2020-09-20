#include <stddef.h>

#include "../../libc.h"
#include "../../syscall.h"

void *
_getheap(void)
{
	return _brk(0);
}
