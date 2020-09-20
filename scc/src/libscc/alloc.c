#include <stdlib.h>
#include <scc/scc.h>

/*
 * This is the most pedantic piece of code that I have written
 * in my life. The next union is used to enforce the alignment
 * of the address returned by new(). A union has the alignment
 * of the field with the biggest alignment. This union has all
 * the types that we use in scc, and we round all the addresses
 * to the alignment of this struct, so we can be sure that any
 * pointer using that address will be safe. The field ap is
 * in the union to be sure that struct pointers are included
 * in the list, although they will have the same alignment or
 * smaller than void *, but I wanted to be pedantic.
 */
union hdr {
	union hdr *next;
	struct arena *ap;
	char c;
	unsigned char uc;
	int i;
	short s;
	long l;
	long long ll;
	float f;
	double d;
	long double ld;
	void *vp;
};

struct arena {
	struct arena *next;
	union hdr *array;
};

struct alloc {
	size_t size;
	size_t nmemb;
	size_t padding;
	struct arena *arena;
	union hdr *freep;
};

static void
newarena(Alloc *allocp)
{
	struct arena *ap;
	union hdr *bp, *lim;
	size_t unit, n = allocp->nmemb;

	unit = (allocp->size-1) / sizeof(union hdr) + 1;
	ap = xmalloc(sizeof(struct arena));
	ap->array = xmalloc(unit * sizeof(union hdr) * n);

	bp = ap->array;
	for (lim = &bp[unit * (n-1)]; bp < lim; bp += unit)
		bp->next = bp + unit;
	bp->next = NULL;

	ap->next = allocp->arena;
	allocp->arena = ap;
	allocp->freep = ap->array;
}

Alloc *
alloc(size_t size, size_t nmemb)
{
	Alloc *allocp = xmalloc(sizeof(*allocp));

	allocp->size = size;
	allocp->nmemb = nmemb;
	allocp->arena = NULL;
	allocp->freep = NULL;

	return allocp;
}

void
dealloc(Alloc *allocp)
{
	struct arena *ap, *next;

	for (ap = allocp->arena; ap; ap = next) {
		next = ap->next;
		free(ap->array);
		free(ap);
	}
	free(allocp);
}

void *
new(Alloc *allocp)
{
	union hdr *bp;

	if (!allocp->freep)
		newarena(allocp);
	bp = allocp->freep;
	allocp->freep = bp->next;

	return bp;
}

void
delete(Alloc *allocp, void *p)
{
	union hdr *bp = p;

	bp->next = allocp->freep;
	allocp->freep = bp;
}
