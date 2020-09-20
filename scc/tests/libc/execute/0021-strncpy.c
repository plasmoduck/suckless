#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SIZ 6

/*
output:
testing
test1
test2
test3
done
end:
*/

int
main()
{
	char *s, buf[SIZ];

	puts("testing");

	puts("test1");
	memset(buf, '0', SIZ);
	s = strncpy(buf, "abc", sizeof(buf));
	assert(s == buf);
	assert(!memcmp(s, (char[SIZ]) {"abc"}, sizeof(buf)));

	puts("test2");
	memset(buf, '0', SIZ);
	s = strncpy(buf, "", sizeof(buf));
	assert(s == buf);
	assert(!memcmp(s, (char[SIZ]) {'\0'}, sizeof(buf)));

	puts("test3");
	memset(buf, '0', SIZ);
	s = strncpy(buf, "", 1);
	assert(s == buf);
	assert(!memcmp(s,
	               (char[SIZ]) {'\0', '0', '0', '0', '0', '0'},
	               sizeof(buf)));

	puts("done");

	return 0;
}
