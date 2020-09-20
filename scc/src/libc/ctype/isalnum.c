#include <ctype.h>
#undef isalnum

int
isalnum(int c)
{
	return (__ctype+1)[c] & (_U|_L|_D);
}
