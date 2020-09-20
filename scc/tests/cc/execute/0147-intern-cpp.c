#define x(y)  (y)

int
main(void)
{
	int y;
	char *p;

	p = __FILE__;
	y = __LINE__;
	p = __DATE__;
	y = __STDC__;
	p = __TIME__;
	y = __STDC_HOSTED__;
	y = __SCC__;
	y = x(1);

	return 0;
}
