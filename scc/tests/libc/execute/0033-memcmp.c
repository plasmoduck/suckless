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
main(void)
{
	char buf[40] = {1, 2, 3, 4, 5};
	signed char buf2[40] = {-127};

	puts("testing");
	assert(memcmp(buf, (char[]) {1, 2, 3, 4, 5}, 5) == 0);
	assert(memcmp(buf, (char[]) {1, 1, 1, 1, 1}, 5) > 0);
	assert(memcmp(buf, (char[]) {1, 3, 1, 1, 1}, 5) < 0);
	assert(memcmp(buf, (char[]) {2, 3, 4, 5, 6}, 0) == 0);
	assert(memcmp(buf2, (char[]) {-127}, 1) == 0);
	puts("done");

	return 0;
}
