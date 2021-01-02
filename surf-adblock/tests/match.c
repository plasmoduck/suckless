#include "../adblock.c"

int
main(void)
{
	int m;

	m = match("a", "a", 1);
	printf("%d = 0\n", m);

	m = match("a*", "a", 1);
	printf("%d = 0\n", m);

	m = match("*a", "a", 1);
	printf("%d = 0\n", m);

	m = match("*a*", "a", 1);
	printf("%d = 0\n", m);

	m = match("^*", "/index.html", 1);
	printf("%d = 0\n", m);

	m = match("*^*", "/index.html", 1);
	printf("%d = 0\n", m);

	m = match("*^*", "a/index.html", 1);
	printf("%d = 0\n", m);

	m = match("*/*", "a/index.html", 1);
	printf("%d = 0\n", m);

	m = match("*^i*", "a/index.html", 1);
	printf("%d = 0\n", m);

	m = match("a^i*", "a/index.html", 1);
	printf("%d = 0\n", m);

	m = match("b^i*", "a/index.html", 1);
	printf("%d = 1\n", m);

	m = match("a^^i*", "a/index.html", 1);
	printf("%d = 1\n", m);

	m = match("^^i*", "a/index.html", 1);
	printf("%d = 1\n", m);

	m = match("^^i*", "a/index.html", 1);
	printf("%d = 1\n", m);

	return 0;
}
