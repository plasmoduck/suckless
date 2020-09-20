static char heap[16384];

void *
_getheap(void)
{
	return heap;
}

void *
_brk(void *addr)
{
	static char *cur = heap;
	char *p = addr;

	if (p < heap || p > &heap[sizeof(heap) - 1])
		return (void *)-1;
	return cur = p;
}
