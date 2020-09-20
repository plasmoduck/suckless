#include <stdio.h>
#undef getchar

int
getchar(void)
{
	return getc(stdin);
}
