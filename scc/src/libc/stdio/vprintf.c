#include <stdarg.h>
#include <stdio.h>
#undef vprintf

int
vprintf(const char * restrict fmt, va_list ap)
{
	va_list ap2;

	va_copy(ap2, ap);
	return vfprintf(stdout, fmt, ap2);
}
