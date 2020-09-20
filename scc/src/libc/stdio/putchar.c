#include <stdio.h>
#undef putchar

int
putchar(int ch)
{
	return putc(ch, stdout);
}
