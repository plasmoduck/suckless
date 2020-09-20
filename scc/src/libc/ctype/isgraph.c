#include <ctype.h>
#undef isgraph

int
isgraph(int c)
{
	return (__ctype+1)[c] & (_P|_U|_L|_D);
}
