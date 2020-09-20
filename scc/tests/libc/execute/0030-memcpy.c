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
	char buf[40];

	puts("testing");
	assert(!memcmp(memcpy(buf, "abc", 3), "abc", 3));
	assert(memcpy(buf, "abc", 0) == buf);
	puts("done");

	return 0;
}
