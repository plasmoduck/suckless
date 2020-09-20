#include <ctype.h>
#undef iscntrl

int
iscntrl(int c)
{
	return (__ctype+1)[c] & (_C);
}
