#include <stdio.h>
#undef feof

int
feof(FILE *fp)
{
	return fp->flags & _IOEOF;
}
