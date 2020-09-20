#include <stdarg.h>
#include <stdio.h>
#undef printf

int
printf(const char * restrict fmt, ...)
{
	int cnt;
	va_list va;

	va_start(va, fmt);
	cnt = vfprintf(stdout, fmt, va);
	va_end(va);
	return cnt;
}
