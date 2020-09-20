#include <signal.h>
#include <stdlib.h>
#undef abort

void
abort(void)
{
	raise(SIGABRT);
	_Exit(127);
}
