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

	assert(strcmp("abcd", "abcd") == 0);
	assert(strcmp("abcd", "a") > 0);
	assert(strcmp("a", "abcd") < 0);
	assert(strcmp("aa", "ab") < 0);
	assert(strcmp("ab", "aa") > 0);
	assert(strcmp("", "a") < 0);
	assert(strcmp("a", "") > 0);

	puts("done");

	return 0;
}
