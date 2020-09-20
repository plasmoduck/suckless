#include <ctype.h>
#undef isdigit

int
isdigit(int c)
{
	return (__ctype+1)[c] & (_D);
}
