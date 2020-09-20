#include <stdio.h>
#undef puts

int
puts(const char *str)
{
	int ch;

	while (ch = *str++)
		putchar(ch);
	return putchar('\n');
}
