#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*
TODO: Test va_copy, new c99 extension.
output:
test 1
test 2
end:
*/

void
test(char *s, char *fmt, ...)
{
	va_list va;
	int cnt = 1;
	long long *p;

	puts(s);

	va_start(va, fmt);
	while (*fmt) {
		printf("fmt = '%c'\n", *fmt);
		switch (*fmt++) {
		case 'c':
			assert(va_arg(va, int) == cnt++);
			break;
		case 's':
			assert(va_arg(va, int) == cnt++);
			break;
		case 'i':
			assert(va_arg(va, int) == cnt++);
			break;
		case 'l':
			assert(va_arg(va, long) == cnt++);
			break;
		case 'q':
			assert(va_arg(va, long long) == cnt++);
			break;
		case 'p':
			p = va_arg(va, void *);
			assert(*p == cnt++);
			break;
		case 'f':
			assert(va_arg(va, double) == cnt++);
			break;
		default:
			abort();
		}
	}
	va_end(va);
}

int
main()
{
	char c;
	short s;
	int i;
	long l;
	long long ll;
	float f;

	c = 1;
	i = 2;
	l = 3;
	ll = 4;
	test("test 1", "cilp", c, i, l, (void *) &ll);


	c = 1;
	s = 2;
	ll = 3;
	f = 4.0;
	test("test 2", "csqf", c, s, ll, f);

	return 0;
}
