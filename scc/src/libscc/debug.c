#include <stdarg.h>
#include <stdio.h>

#include <scc/scc.h>

int enadebug;

void
dbg(const char *fmt, ...)
{
	if (!enadebug)
		return;
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	putc('\n', stderr);
	va_end(va);
	return;
}
