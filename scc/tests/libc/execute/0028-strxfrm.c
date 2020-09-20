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
	size_t n;

	puts("testing");

	assert(strxfrm(buf, "test", 40) == 4);
	assert(strxfrm(buf, "", 0) == 0);
	assert(strxfrm(NULL, "abc", 0) > 0);

	puts("done");

	return 0;
}
