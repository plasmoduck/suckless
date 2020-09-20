#ifndef _TIME_H
#define _TIME_H

#define _NEED_SIZET
#define _NEED_NULL
#include <sys/cdefs.h>
#include <arch/cdefs.h>
#include <arch/time.h>

#define CLOCKS_PER_SEC 1000000

typedef long int clock_t;

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;

	/* fields used internally */

	char *tm_zone;
	long tm_gmtoff;
};

extern clock_t clock(void);
extern double difftime(time_t time1, time_t time0);
extern time_t mktime(struct tm *timeptr);
extern time_t time(time_t *timer);
extern char *asctime(const struct tm *timeptr);
extern char *ctime(const time_t *timer);
extern struct tm *gmtime(const time_t *timer);
extern struct tm *localtime(const time_t *timer);
extern size_t strftime(char * restrict s, size_t maxsize,
                       const char * restrict format,
                       const struct tm * restrict timeptr);

#endif
