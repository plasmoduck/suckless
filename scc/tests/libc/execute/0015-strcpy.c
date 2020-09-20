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
	char *s, buf[40];

	puts("testing");

	s = strcpy(buf, "test");
	assert(s == buf);
	assert(!strcmp(s, "test"));

	s = strcpy(buf, "");
	assert(s == buf);
	assert(!strcmp(s, ""));

	puts("done");

	return 0;
}
