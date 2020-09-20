#include <ctype.h>
#undef islower

int
islower(int c)
{
	return (__ctype+1)[c] & _L;
}
