#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../libc.h"

#undef strtol

long
strtol(const char * restrict s, char ** restrict end, int base)
{
	int d, sign = -1;
	long n;
	const char *t;

	if (end)
		*end = s;

	while (isspace(*s))
		++s;

	switch (*s) {
	case '-':
		sign = 1;
	case '+':
		++s;
	}

	if (base == 0) {
		if (*s == '0')
			base = toupper(s[1]) == 'X' ? 16 : 8;
		else
			base = 10;
	}
	if (base == 16 && *s == '0' && toupper(s[1]) == 'X')
		s += 2;

	n = 0;
	/* Compute n as a negative number to avoid overflow on LONG_MIN */
	for (t = s; (d = _dtoi(*t)) < base; ++t) {
		if (n < LONG_MIN/base)
			goto overflow;
		n *= base;
		if (-d < LONG_MIN - n)
			goto overflow;
		n -= d;
		if (n == LONG_MIN && sign < 0)
			goto overflow;
	}

	if (end && t != s)
		*end = t;

	return n*sign;

overflow:
	if (end)
		*end = t;
	errno = ERANGE;

	return sign < 0 ? LONG_MAX : LONG_MIN;
}
