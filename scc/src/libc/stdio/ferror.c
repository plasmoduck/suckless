#include <stdio.h>
#undef ferror

int
ferror(FILE *fp)
{
	return fp->flags & _IOERR;
}
