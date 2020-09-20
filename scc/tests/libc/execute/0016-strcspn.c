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

	assert(strcspn("0125", "56789") == 3);
	assert(strcspn("", "56789") == 0);
	assert(strcspn("01234", "") == 5);
	assert(strcspn("", "") == 0);

	puts("done");

	return 0;
}
