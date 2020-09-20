struct dummy;

void *
fun(struct dummy p[])
{
	return p;
}

int
main()
{
	return fun(0) != 0;
}
