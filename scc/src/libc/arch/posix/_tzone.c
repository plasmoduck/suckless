#include <stdlib.h>
#include <time.h>
#include "../../libc.h"

struct tzone *
_tzone(struct tm *tm)
{
	static struct tzone tz;
	static int first = 1;

	if (!first)
		return &tz;

	tz.name = getenv("TZ");
	if (!tz.name || *tz.name == '\0') {
		tz.name = NULL;
		tz.gmtoff = 0;
		tz.isdst = 0;
	} else {
		/* TODO: parse TZ string */
		tz.gmtoff = 0;
		tz.isdst = 0;
	}
	first = 0;

	return &tz;
}
