#include <ctype.h>
#undef toupper

int
toupper(int c)
{
	return (islower(c)) ? c & ~0x20 : c;
}
