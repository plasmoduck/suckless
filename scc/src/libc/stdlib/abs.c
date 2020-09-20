#include <stdlib.h>
#undef abs

int
abs(int n)
{
	return (n < 0) ? -n : n;
}
