#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#undef vsprintf


int
vsprintf(char * restrict s, const char * restrict fmt, va_list va)
{
	return vsnprintf(s, SIZE_MAX, fmt, va);
}
