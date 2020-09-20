#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
output:
testing
done
end:
*/

int
main()
{
	char buf[] = {0, 1, 2, 3, 4, 160};

	puts("testing");
	assert(memchr(buf, 2, 6) == buf+2);
	assert(memchr(buf, 2, 0) == NULL);
	assert(memchr(buf, 150, 6) == NULL);
	assert(memchr(buf, 160, 6) == buf+5);
	puts("done");

	return 0;
}
