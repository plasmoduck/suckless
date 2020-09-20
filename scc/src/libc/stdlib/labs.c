#include <stdlib.h>
#undef labs

long
labs(long n)
{
	return (n < 0) ? -n : n;
}
