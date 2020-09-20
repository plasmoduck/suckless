#include <time.h>
#include "../libc.h"

int _daysmon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int
_daysyear(int year)
{
	if (year%4 != 0)
		return 365;
	if (year%100 == 0 && year%400 != 0)
		return 365;
	return 366;
}

/*
 * Happy New Year!!!!
 */
int
_newyear(int year)
{
	int day;

	year += 1900 - 1;
	day = 1 + year + year/4;
	day -= year/100;
	day += year/400;

	return day % 7;
}
