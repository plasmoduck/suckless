#include <assert.h>
#include <locale.h>
#include <stdio.h>

/*
output:
testing
done
end:
*/

int
main()
{
	puts("testing");
	assert(!strcmp(setlocale(LC_ALL, NULL), "C"));
	assert(!strcmp(setlocale(LC_COLLATE, NULL), "C"));
	assert(!strcmp(setlocale(LC_CTYPE, NULL), "C"));
	assert(!strcmp(setlocale(LC_MONETARY, NULL), "C"));
	assert(!strcmp(setlocale(LC_NUMERIC, NULL), "C"));
	assert(!strcmp(setlocale(LC_TIME, NULL), "C"));

	assert(!strcmp(setlocale(LC_ALL, "C"), "C"));
	assert(!strcmp(setlocale(LC_COLLATE, "C"), "C"));
	assert(!strcmp(setlocale(LC_CTYPE, "C"), "C"));
	assert(!strcmp(setlocale(LC_MONETARY, "C"), "C"));
	assert(!strcmp(setlocale(LC_NUMERIC, "C"), "C"));
	assert(!strcmp(setlocale(LC_TIME, "C"), "C"));

	assert(!setlocale(LC_ALL, "invalid-locale"));
	assert(!setlocale(LC_COLLATE, "invalid-locale"));
	assert(!setlocale(LC_CTYPE, "invalid-locale"));
	assert(!setlocale(LC_MONETARY, "invalid-locale"));
	assert(!setlocale(LC_NUMERIC, "invalid-locale"));
	assert(!setlocale(LC_TIME, "invalid-locale"));

	assert(setlocale(LC_ALL, ""));
	assert(setlocale(LC_COLLATE, ""));
	assert(setlocale(LC_CTYPE, ""));
	assert(setlocale(LC_MONETARY, ""));
	assert(setlocale(LC_NUMERIC, ""));
	assert(setlocale(LC_TIME, ""));
	puts("done");

	return 0;
}
