#include <stdio.h>
#undef fgetc

int
fgetc(FILE *fp)
{
	return getc(fp);
}
