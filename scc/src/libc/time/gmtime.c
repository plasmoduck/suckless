#include <time.h>

#include "../libc.h"
#undef gmtime

struct tm *
gmtime(const time_t *t)
{
        static struct tm tm;
        time_t sec, min, hour, year, day;
	int i;

        tm.tm_sec = *t % SECDAY;
	tm.tm_min = tm.tm_sec / 60;
	tm.tm_sec %= 60;
	tm.tm_hour = tm.tm_min / 60;
	tm.tm_min %= 60;
	day = *t / SECDAY;

	tm.tm_wday = (day + THU) % 7; /* 1/1/1970 was Thursday */

	for (i = EPOCH; day >= _daysyear(i); ++i)
		day -= _daysyear(i);
        tm.tm_year = i - 1900;
	tm.tm_yday = day;

	_daysmon[FEB] = FEBDAYS(tm.tm_year);
	for (i = JAN; day > _daysmon[i]; i++)
		day -= _daysmon[i];
	tm.tm_mon = i;
	tm.tm_mday = day + 1;

	tm.tm_isdst = 0;

        return &tm;
}
