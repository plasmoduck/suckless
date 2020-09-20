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
	char *p, buf[] = "abcad";

	puts("testing");

	p = strchr(buf, 0);
	assert(p == buf+5);
	assert(*p == '\0');

	p = strchr(buf, 'a');
	assert(p == buf);
	assert(*p == 'a');

	p = strchr(buf, 'd');
	assert(p == buf+4);
	assert(*p == 'd');

	p = strchr(buf, 'c');
	assert(p == buf+2);
	assert(*p == 'c');

	p = strchr(buf, 'h');
	assert(p == NULL);

	p = strchr("", 'a');
	assert(p == NULL);

	puts("done");

	return 0;
}
