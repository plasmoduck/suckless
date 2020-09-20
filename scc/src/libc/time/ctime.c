#include <time.h>
#undef ctime

char *
ctime(const time_t *t)
{
	return asctime(localtime(t));
}
