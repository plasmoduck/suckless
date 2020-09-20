#include <stdarg.h>
#include <stdio.h>
#undef vsnprintf

int
vsnprintf(char * restrict s, size_t siz, const char * restrict fmt, va_list ap)
{
	FILE f;
	int r;

	f.flags = _IORW | _IOSTRG;
	f.len = siz;
	f.buf = s;
	f.wp = s;
	f.rp = s + siz;

	r = vfprintf(&f, fmt, ap);
	if (s) {
		if (f.wp == f.rp)
			--f.wp;
		*f.wp = '\0';
	}

	return r;
}
