#include <stdarg.h>
#include <stdio.h>
#undef snprintf

int
snprintf(char * restrict s, size_t siz, const char * restrict fmt, ...)
{
	int r;
	va_list va;

	va_start(va, fmt);
	r = vsnprintf(s, siz, fmt, va);
	va_end(va);

	return r;
}
