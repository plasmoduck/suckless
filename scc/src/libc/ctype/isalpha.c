#include <ctype.h>
#undef isalpha

int
isalpha(int c)
{
	return (__ctype+1)[c] & (_U|_L);
}
