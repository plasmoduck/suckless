#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
output:
testing
done
end:
*/

int
main()
{
	char *p, buf[40];

	puts("testing");

	memset(buf, 2, sizeof(buf));
	for (p = buf; p < &buf[40]; ++p)
		assert(*p == 2);

	memset(buf, 0, 0);
	for (p = buf; p < &buf[40]; ++p)
		assert(*p == 2);

	puts("done");

	return 0;
}
