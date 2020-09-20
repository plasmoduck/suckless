#include <time.h>
#undef difftime

double
difftime(time_t t1, time_t t2)
{
	return (double) (t1 - t2);
}
