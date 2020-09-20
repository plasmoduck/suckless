#include <setjmp.h>
#include <stdio.h>

/*
output:
test 1
i = 1, v = 2
test 2
i = 2, v = 3
end:
*/

jmp_buf recover;

void
test(char *s, int val)
{
	puts(s);
	longjmp(recover, val);
}

int
main()
{
	static int i;
	auto volatile int v;

	i = 0;
	v = 1;
	if (!setjmp(recover)) {
		i = 1;
		v = 2;
		test("test 1", 1);	
	}
	printf("i = %d, v = %d\n", i, v);

	if (!setjmp(recover)) {
		i = 2;
		v = 3;
		test("test 2", 0);
	}
	printf("i = %d, v = %d\n", i, v);

	return 0;
} 
