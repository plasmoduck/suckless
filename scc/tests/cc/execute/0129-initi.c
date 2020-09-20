struct range {
	long quant;
} *a;
long b;

int
main()
{
	struct range r = a[0];
	b = r.quant;
}
