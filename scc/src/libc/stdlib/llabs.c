#include <stdlib.h>
#undef llabs

long long
llabs(long long n)
{
	return (n < 0) ? -n : n;
}
