#include <ctype.h>
#include <limits.h>
#include <string.h>

int
_dtoi(char c)
{
	static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *p;

	if (p = strchr(digits, toupper(c)))
		return p - digits;

	return INT_MAX;
}
