#include <ctype.h>
#undef toascii

int
toascii(int c)
{
	return c & 0x7f;
}
