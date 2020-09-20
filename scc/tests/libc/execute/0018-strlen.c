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

	assert(strlen("01234") == 5);
	assert(strlen("") == 0);

	puts("done");

	return 0;
}
