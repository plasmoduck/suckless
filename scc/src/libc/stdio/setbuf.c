#include <stdio.h>
#undef setbuf

void
setbuf(FILE * restrict fp, char * restrict buf)
{
	setvbuf(fp, buf, (buf) ? _IOFBF : _IONBF, BUFSIZ);
}
