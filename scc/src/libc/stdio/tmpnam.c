#include <stdio.h>
#include <string.h>
#include "../syscall.h"
#undef tmpnam

char *
tmpnam(char *s)
{
	static char *tmpl, buf[L_tmpnam];
	char *p;

	if (*buf == '\0') {
		for (tmpl = buf, p = _TMPNAME; *tmpl++ = *p++; )
			;
		for (p = tmpl; p < &buf[L_tmpnam-1]; ++p)
			*p = '0';
		*p = '\0';
	}
	for (;;) {
		for (p = tmpl; *p && *p != '9'; ++p)
			;
		if (*p == '\0')
			return NULL;
		++*p;
		if (_access(buf, 0) != 0)
			break;
	}
	if (s)
		strcpy(s, buf);
	return buf;
}
