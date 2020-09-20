#include <time.h>
#undef asctime

char *
asctime(const struct tm *tm)
{
	static char buf[30];

	strftime(buf, sizeof(buf), "%c\n", tm);
	return buf;
}
