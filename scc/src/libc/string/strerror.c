#include <errno.h>
#include <string.h>
#undef strerror

char *
strerror(int errnum)
{
	if (errnum > _sys_nerr)
		errnum = EUNKNOWN;
	return _sys_errlist[errnum];
}
