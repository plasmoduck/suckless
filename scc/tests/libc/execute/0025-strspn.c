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
	assert(strspn("abccdef", "cba") == 4);
	assert(strspn("abcg", "cba0") == 3);
	assert(strspn("", "abc") == 0);
	assert(strspn("abc", "") == 0);
	assert(strspn("", "") == 0);
	assert(strspn("abc", "def") == 0);
	puts("done");

	return 0;
}
