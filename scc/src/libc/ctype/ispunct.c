#include <ctype.h>
#undef ispunct

int
ispunct(int c)
{
	return (__ctype+1)[c] & (_P);
}
