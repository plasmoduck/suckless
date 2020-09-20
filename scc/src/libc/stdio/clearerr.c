#include <stdio.h>
#undef clearerr

void
clearerr(FILE *fp)
{
	fp->flags &= ~(_IOERR | _IOEOF);
}
