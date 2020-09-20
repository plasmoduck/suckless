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
	char buf[] = "abcdef";

	puts("testing");
	assert(strpbrk(buf, "a") == buf);
	assert(strpbrk(buf, "") == NULL);
	assert(strpbrk("", "a") == NULL);
	assert(strpbrk("", "") == NULL);
	assert(strpbrk(buf, "1234") == NULL);
	assert(strpbrk(buf, "f") == buf + 5);
	assert(strpbrk(buf, "12345a") == buf);
	puts("done");

	return 0;
}
