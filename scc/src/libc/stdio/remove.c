#include <stdio.h>

#include "../syscall.h"

#undef remove

int
remove(const char *filename)
{
	return _unlink(filename);
}
