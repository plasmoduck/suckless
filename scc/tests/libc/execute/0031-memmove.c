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
	char buf[30];

	puts("testing");

	memcpy(buf, "abcdef", 6);
	assert(!memcmp(memmove(buf, buf+3, 3), "def", 3));
	memcpy(buf, "abcdef", 6);
	assert(!memcmp(memmove(buf, buf+3, 0), "abc", 3));

	puts("done");

	return 0;
}
