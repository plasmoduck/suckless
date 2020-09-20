#include <limits.h>
#include <stdio.h>

/*
 * This test assumes that CHAR_BIT is the size of every
 * unit returned by sizeof. It also assumes 2 complement.
 */

/*
output:
test1
test2
test3
end:
*/

void
test1()
{
	puts("test1");

	if (CHAR_BIT < 8 ||
	    CHAR_MAX < 127 || CHAR_MIN > 0 ||
	    CHAR_MAX != SCHAR_MAX && CHAR_MAX != UCHAR_MAX)
		puts("wrong char definition");

	if (SCHAR_MAX < 127 || CHAR_MIN > -127)
		puts("wrong signed char definition");

	if (UCHAR_MAX < 255 || UCHAR_MAX <= 0)
		puts("wrong unsigned char definition");

	if (SHRT_MAX < 32767 ||
	    SHRT_MIN > -32767 ||
	    USHRT_MAX < 65535 || USHRT_MAX <= 0)
		puts("wrong short definition");

	if (INT_MAX < 32767 ||
	    INT_MIN > -32767 ||
	    UINT_MAX < 65535 || UINT_MAX <= 0 ||
	    INT_MAX < SCHAR_MAX || INT_MIN > SCHAR_MIN ||
	    UINT_MAX < UCHAR_MAX ||
	    INT_MAX < SHRT_MAX || INT_MIN > SHRT_MIN ||
	    UINT_MAX < USHRT_MAX)
		puts("wrong int definition");

	if (LONG_MAX < 2147483647 ||
	    LONG_MIN > -2147483647 ||
	    ULONG_MAX < 4294967295 || ULONG_MAX <= 0 ||
	    LONG_MAX < SCHAR_MAX || LONG_MIN > SCHAR_MIN ||
	    ULONG_MAX < UCHAR_MAX ||
	    LONG_MAX < SHRT_MAX || LONG_MIN > SHRT_MIN ||
	    ULONG_MAX < USHRT_MAX ||
	    LONG_MAX < INT_MAX || LONG_MIN > INT_MIN ||
	    ULONG_MAX < UINT_MAX)
		puts("wrong long definition");

	if (LONG_MAX < 9223372036854775807 ||
	    LONG_MIN > -9223372036854775807 ||
	    ULONG_MAX < 18446744073709551615 || ULONG_MAX <= 0 ||
	    LONG_MAX < SCHAR_MAX || LONG_MIN > SCHAR_MIN ||
	    ULONG_MAX < UCHAR_MAX ||
	    LONG_MAX < SHRT_MAX || LONG_MIN > SHRT_MIN ||
	    ULONG_MAX < USHRT_MAX ||
	    LONG_MAX < LONG_MAX || LONG_MIN > LONG_MIN ||
	    ULONG_MAX < ULONG_MAX)
		puts("wrong long definition");

	if (LLONG_MAX < 9223372036854775807 ||
	    LLONG_MIN > -9223372036854775807 ||
	    ULLONG_MAX < 18446744073709551615 || ULLONG_MAX <= 0 ||
	    LLONG_MAX < SCHAR_MAX || LLONG_MIN > SCHAR_MIN ||
	    ULLONG_MAX < UCHAR_MAX ||
	    LLONG_MAX < SHRT_MAX || LLONG_MIN > SHRT_MIN ||
	    ULLONG_MAX < USHRT_MAX ||
	    LLONG_MAX < LONG_MAX || LLONG_MIN > LONG_MIN ||
	    ULLONG_MAX < ULONG_MAX)
		puts("wrong long long definition");

	if (MB_LEN_MAX < sizeof(char))
		puts("wrong value for MB_LEN_MAX");
}


void
test2()
{
	char c;
	int i;

	puts("test2");
	if ('\xff' > 0) {
		for (c = i = 0; i < CHAR_BIT; i++) {
			c <<= 1;
			c |= 1;
		}
		if (c != CHAR_MAX)
			printf("wrong char max %d-%d", c, CHAR_MAX);
		if (CHAR_MIN != 0)
			printf("wrong char min %d-%d", c, CHAR_MIN);
	} else {
		for (c = i = 0; i < CHAR_BIT -1; i++) {
			c <<= 1;
			c |= 1;
		}
		if (c != CHAR_MAX)
			printf("wrong char max %d-%d", c, CHAR_MAX);
		c = -c - 1;
		if (c != CHAR_MIN)
			printf("wrong char min %d-%d", c, CHAR_MIN);
	}
}

#define SMAX(t) for (t = n = 0; n < sizeof(t)*CHAR_BIT -1; n++) {t <<= 1; t |= 1;}
#define UMAX(t) for (t = n = 0; n < sizeof(t)*CHAR_BIT; n++) {t <<= 1; t |= 1;}

void
test3()
{
	signed char sc;
	unsigned char uc;
	int i, n;
	unsigned u;
	long l;
	unsigned long ul;
	long long ll;
	unsigned long long ull;

	puts("test3");
	SMAX(sc);
	if (sc != SCHAR_MAX)
		printf("wrong signed char max %d %d\n", sc, SCHAR_MAX);
	sc = -sc - 1;
	if (sc != SCHAR_MIN)
		printf("wrong signed char min %d %d\n", sc, SCHAR_MIN);

	UMAX(uc);
	if (uc != UCHAR_MAX)
		printf("wrong unsigned char max %u %u", uc, UCHAR_MAX);

	SMAX(i);
	if (i != INT_MAX)
		printf("wrong int max %d %d\n", i, INT_MAX);
	i = -i - 1;
	if (i != INT_MIN)
		printf("wrong int min %d %d\n", i, INT_MIN);

	UMAX(u);
	if (u != UINT_MAX)
		printf("wrong unsigned int max %u %u\n", u, UINT_MAX);

	SMAX(l);
	if (l != LONG_MAX)
		printf("wrong long max %ld %ld\n", l, (long) LONG_MAX);
	l = -l - 1;
	if (l != LONG_MIN)
		printf("wrong long max %ld %ld\n", l, (long) LONG_MIN);

	UMAX(ul);
	if (ul != ULONG_MAX)
		printf("wrong int max %lu %lu\n", ul, (unsigned long) ULONG_MAX);

	SMAX(ll);
	if (ll != LLONG_MAX)
		printf("wrong llong max %lld %lld\n", ll, (long long) LLONG_MAX);
	ll = -ll - 1;
	if (ll != LLONG_MIN)
		printf("wrong llong min %lld %lld\n", ll, (long long) LLONG_MIN);

	UMAX(ull);
	if (ull != ULLONG_MAX)
		printf("wrong ullong max %llu %llu\n", ull, (unsigned long long) ULLONG_MAX);
}

int
main()
{
	test1();
	test2();
	test3();

	return 0;
}
