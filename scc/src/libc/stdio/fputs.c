#include <stdio.h>
#undef fputs

int
fputs(const char * restrict bp, FILE * restrict fp)
{
	int r, ch;

	while (ch = *bp++)
		r = putc(ch, fp);
	return r;
}
