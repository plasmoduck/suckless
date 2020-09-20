#include <stddef.h>

#include "../../../syscall.h"

#define AT_FDCWD  -100

extern int _openat(int fd, const char *fname, int flags, int mode);

int
_open(const char *fname, int flags, int mode)
{
	return _openat(AT_FDCWD, fname, flags, mode);
}
