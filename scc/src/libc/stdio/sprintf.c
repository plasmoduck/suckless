#include <stdarg.h>
#include <stdio.h>
#undef sprintf

int
sprintf(char * restrict s, const char * restrict fmt, ...)
{
	int r;

	va_list va;
	va_start(va, fmt);
	r = vsprintf(s, fmt, va);
	va_end(va);

	return r;
}
