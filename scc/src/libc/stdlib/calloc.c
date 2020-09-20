#include <stdlib.h>
#include <string.h>
#undef calloc

void *
calloc(size_t nmemb, size_t size)
{
	size_t nbytes;
	void *mem;

	if (!nmemb || !size || nmemb > (size_t)-1/size)
                return NULL;

	nbytes = nmemb * size;
	if ((mem = malloc(nbytes)) == NULL)
		return NULL;
	return memset(mem, 0, nbytes);
}
