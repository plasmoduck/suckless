#include <stdio.h>
#undef fread

size_t
fread(void * restrict ptr, size_t size, size_t nmemb,
      FILE * restrict fp)
{
	unsigned char *bp = ptr;
	size_t n, i;
	int c;

	if (size == 0)
		return 0;

	for (n = 0; n < nmemb; n++) {
		i = size;
		do {
			if ((c = getc(fp)) == EOF)
				return n;
			*bp++ = c;
		} while (--i);
	}

	return n;
}
