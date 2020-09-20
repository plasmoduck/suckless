#include <ctype.h>
#undef isprint

int
isprint(int c)
{
	return (__ctype+1)[c] & (_P|_U|_L|_D|_SP);
}
