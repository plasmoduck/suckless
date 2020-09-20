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
	char buf[] = "012321";

	puts("testing");
	assert(strrchr(buf, '1') == buf+5);
	assert(strrchr(buf, '0') == buf);
	assert(strrchr(buf, '3') == buf+3);
	assert(strrchr("",  '0') == NULL);
	assert(strrchr(buf, 'a')  == NULL);
	assert(strrchr(buf, 0) == buf+6);
	puts("done");

	return 0;
}
