#include <assert.h>
#include <stdio.h>
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

	assert(strcoll("abcd", "abcd") == 0);
	assert(strcoll("abcd", "a") > 0);
	assert(strcoll("a", "abcd") < 0);
	assert(strcoll("aa", "ab") < 0);
	assert(strcoll("ab", "aa") > 0);
	assert(strcoll("", "a") < 0);
	assert(strcoll("a", "") > 0);

	puts("done");

	return 0;
}
