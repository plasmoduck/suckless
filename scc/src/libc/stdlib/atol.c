#include <ctype.h>
#include <stdlib.h>

#include "../libc.h"

#undef atol

long
atol(const char *s)
{
	int d, sign = -1;
	long n;

	while (isspace(*s))
		++s;

	switch (*s) {
	case '-':
		sign = 1;
	case '+':
		++s;
	}

	/* Compute n as a negative number to avoid overflow on LONG_MIN */
	for (n = 0; (d = _dtoi(*s)) < 10; ++s)
		n = n*10 - d;

	return sign * n;
}
