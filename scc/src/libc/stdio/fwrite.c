#include <stdio.h>
#undef fwrite

size_t
fwrite(const void * restrict ptr, size_t size, size_t nmemb,
       FILE * restrict fp)
{
	const unsigned char *bp = ptr;
	size_t n, i;

	if (size == 0)
		return 0;

	for (n = 0; n < nmemb; n++) {
		i = size;
		do
			putc(*bp++, fp);
		while (--i);
		if (ferror(fp))
			break;
	}

	return n;
}
