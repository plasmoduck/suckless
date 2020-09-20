#include <assert.h>
#include <stddef.h>
#include <stdio.h>

/*
output:
NULL = 0
end:
*/

typedef struct test Test;

struct test {
	int a, b, c;
	char d;
} test = {
	.a = 1,
	.b = 2,
	.c = 3,
	.d = 4,
};

int
main()
{
	wchar_t wc = L'c';
	char *q, *p = (char *) &test;

	printf("NULL = %p\n", NULL);

	p += offsetof(Test, d);

	assert(sizeof(size_t) == sizeof(ptrdiff_t));
	assert(wc == L'c');
	assert(*p == 4);
	assert(offsetof(Test, d) > offsetof(Test, a));
	assert(offsetof(Test, d) > offsetof(Test, b));
	assert(offsetof(Test, d) > offsetof(Test, c));
	assert(sizeof(sizeof(int)) == sizeof(size_t));
	assert(sizeof(p - q) == sizeof(ptrdiff_t));

	return 0;
}
