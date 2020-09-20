#include <assert.h>
#include <errno.h>
#include <stdio.h>

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
	assert(errno == 0);
	assert(ERANGE != EDOM);
	assert(ERANGE > 0);
	assert(EDOM > 0);
	puts("done");

	return 0;
}
