#include <time.h>

struct timeval {
	time_t tv_sec;
	int tv_usec; /* TODO use a arch type */
};

int
_gettimeofday(struct timeval * restrict tp, void * restrict tzp);

time_t
time(time_t *t)
{
	struct timeval tv;

	if (_gettimeofday(&tv, NULL) == -1)
		return -1;
	if (t)
		*t =tv.tv_sec;
	return tv.tv_sec;
}
