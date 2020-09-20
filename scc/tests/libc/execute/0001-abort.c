#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

/*
output:
aborting
end:
*/

void
handler(int dummy)
{
	_Exit(0);
}

int
main(void)
{
	printf("aborting\n");
	assert(signal(SIGABRT, handler) != SIG_ERR);
	abort();
	printf("borning\n");

	return 0;
}
