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
	puts("testing");
	assert(strnlen("", 2) == 0);
	assert(strnlen("abc", 10) == 3);
	assert(strnlen((char[3]) {"abc"}, 3) == 3);
	assert(strnlen("abc", 2) == 2);
	puts("done");

	return 0;
}
