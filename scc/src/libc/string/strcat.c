#include <string.h>
#undef strcat

char *
strcat(char * restrict dst, const char * restrict src)
{
	char *ret = dst;

	while (*dst)
		++dst;
	while (*dst++ = *src++)
		;
	return ret;
}
