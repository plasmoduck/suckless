#include <stddef.h>
#include <signal.h>
#include <sys.h>

#undef raise

int
raise(int signum)
{
	return _kill(_getpid(), signum);
}
