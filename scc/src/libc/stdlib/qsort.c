#include <stdlib.h>
#include <string.h>
#undef qsort

/*
 * This implementation of qsort is based in the paper
 * "Engineering a Sort Function", by Jon L.Bentley and M. Douglas McIlroy.
 * A lot of different optimizations were removed to make the code simpler.
 */

struct qsort {
	size_t es;
	int (*cmp)(const void *, const void *);
};

static void
swap(char *i, char *j, size_t n)
{
	do {
		char c = *i;
		*i++ = *j;
		*j++ = c;
	} while (--n > 0);
}

/*
 * This function recurses as much as log2(n) because
 * it always recurses in the smaller part of the
 * array.
 */
static void
xqsort(char *a, size_t n, struct qsort *qs)
{
	size_t nj, ni, es = qs->es;
	char *pi, *pj, *pn;

	if (n <= 1)
		return;

	pi = a;
	pn = pj = a + n*es;

	swap(a, a + n/2 * es,  es);
	for (;;) {
		do {
			pi += es;
		} while  (pi < pn && qs->cmp(pi, a) < 0);

		do {
			pj -= es;
		} while (pj > a && qs->cmp(pj, a) > 0);

		if (pj < pi)
			break;
		swap(pi, pj, es);
	}
	swap(a, pj, es);

	pi = a;
	ni = (pj - a) / es;
	pj += es;
	nj = n-nj-1;

	if (ni < nj) {
		xqsort(pi, ni, qs);
		xqsort(pj, nj, qs);
	} else {
		xqsort(pj, nj, qs);
		xqsort(pi, ni, qs);
	}
}

void
qsort(void *base, size_t nmemb, size_t size,
      int (*f)(const void *, const void *))
{
	struct qsort qs;

	qs.cmp = f;
	qs.es = size;
	xqsort(base, nmemb, &qs);
}
