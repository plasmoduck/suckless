#include <ctype.h>
#undef isxdigit

int
isxdigit(int c)
{
	return (__ctype+1)[c] & (_D|_X);
}
