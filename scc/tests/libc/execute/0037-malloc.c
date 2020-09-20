#include <assert.h>
#include <stdlib.h>

/*
output:

end:
*/

int
check_test1(void **vec)
{
	int i, cnt;

	for (i = cnt = 0; i < 100; i++) {
		if (vec[i])
			cnt++;
	}
	return cnt;
}

void
test()
{
	int i,n, which, nalloc;
	static void *p[100];

	nalloc = 0;
	for (i = 0; i < 5000; i++) {
		which = rand() % 2;
		if (which == 0 && nalloc < 100) {
			for (n = rand()%100; p[n]; n = rand()%100)
				;
			p[n] = malloc(rand() + 1);
			nalloc++;
		} else if (nalloc > 0) {
			n = rand() % 100;
			if (p[n])
				nalloc--;
			free(p[n]);
			p[n] = NULL;
		}
		assert(check_test1(p) == nalloc);
	}

	for (i = 0; i < nalloc; i++) {
		for (n = 0; n < 100 && !p[n]; n++)
			;
		assert(n < 100);
		free(p[n]);
		p[n] = NULL;
	}
	assert(check_test1(p) == 0);

	for (i = 0; i < 100; i++)
		assert(p[i] == NULL);
	/* TODO: Add a verifier here */
}

int
main()
{
	puts("testing");
	test();
	puts("done");

	return 0;
}
