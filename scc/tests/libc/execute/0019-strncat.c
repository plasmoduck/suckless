#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
output:
testing
test1
test2
test3
test4
test5
done
end:
*/

int
main()
{
	char *s, buf[40], buf2[40];

	puts("testing");

	puts("test1");
	strcpy(buf, "01234");
	s = strncat(buf, "567", 8);
	assert(s == buf);
	assert(!strcmp(s, "01234567"));

	puts("test2");
	strcpy(buf, "01234");
	s = strncat(buf, "567", 2);
	assert(s == buf);
	assert(!strcmp(s, "0123456"));

	puts("test3");
	strcpy(buf, "01234");
	memcpy(buf2, "567", 3);
	s = strncat(buf, buf2, 3);
	assert(s == buf);
	assert(!strcmp(s, "01234567"));

	puts("test4");
	strcpy(buf, "01234");
	s = strncat(buf, "", 7);
	assert(s == buf);
	assert(!strcmp(s, "01234"));

	puts("test5");
	strcpy(buf, "01234");
	s = strncat(buf, "", 0);
	assert(s == buf);
	assert(!strcmp(s, "01234"));

	puts("done");

	return 0;
}
