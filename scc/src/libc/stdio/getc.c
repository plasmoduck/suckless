#include <stdio.h>
#undef getc

int
getc(FILE *fp)
{
	return (fp->rp >= fp->wp) ?  __getc(fp) : *fp->rp++;
}
