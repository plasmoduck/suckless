#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
output:
testing
test1
one
two
three
four
test2
one
three
test3
one
done
end:
*/

void
test(char *msg, char *fmt)
{
	char *s, buff[50];

	puts(msg);

	strcpy(buff, fmt);
	for (s = strtok(buff, "-+"); s; s = strtok(NULL, "+-")) {
		switch (atoi(s)) {
		case 1:
			puts("one");
			break;
		case 2:
			puts("two");
			break;
		case 3:
			puts("three");
			break;
		case 4:
			puts("four");
			break;
		default:
			puts("error");
			break;
		}
	}
}

int
main()
{
	puts("testing");
	test("test1", "-+001--0002++3+-4");
	test("test2", "001--+-+-+-3+-");
	test("test3", "001");
	puts("done");
	return 0;
}
