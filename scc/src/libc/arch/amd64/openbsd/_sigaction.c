#include <stddef.h>
#include <sys.h>

extern int _sigaction2(int sig,
                       struct sigaction *new, struct sigaction *old,
                       int siginfo[], int num);

int
_sigaction(int sig, struct sigaction *new, struct sigaction *old)
{
	extern int _setcontext[];

	return _sigaction2(sig, new, old, _setcontext, 2);
}
