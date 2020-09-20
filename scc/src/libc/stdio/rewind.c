#include <stdio.h>
#undef rewind

void
rewind(FILE *fp)
{
	fp->flags &= ~_IOERR;
	fseek(fp, 0, SEEK_SET);
	clearerr(fp);
}
