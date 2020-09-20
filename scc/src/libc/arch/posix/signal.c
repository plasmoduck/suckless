#include <stddef.h>
#include <signal.h>
#include <sys.h>
#undef signal

void
(*signal(int signum, void (*func)(int)))(int)
{
	struct sigaction sa = {
		.sa_handler = func,
	};

	if (_sigaction(signum, &sa, &sa) < 0)
		return SIG_ERR;

	return sa.sa_handler;
}
