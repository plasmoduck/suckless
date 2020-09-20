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
	char buf[30] = "abc";

	puts("testing");
	assert(strstr(buf, "abc") == buf);
	assert(strstr(buf, "bc") == buf + 1);
	assert(strstr(buf, "c") == buf + 2);
	assert(strstr(buf, "d") == NULL);
	strcpy(buf, "ababc");
	assert(strstr(buf, "abc") == buf+2);
	assert(strstr("", "abc") == NULL);
	assert(strstr("abc", "") == NULL);
	assert(strstr("", "") == NULL);
	puts("done");

	return 0;
}

