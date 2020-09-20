#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

sig_atomic_t abrt, fpe, iint, segv, term, def;
jmp_buf recover;

/*
output:
test 1
test 2
test 3
test 4
end:
*/

void
handler1(int sig)
{
	switch (sig) {
	case SIGABRT:
		abrt = 1;
		break;
	case SIGFPE:
		fpe = 1;
		break;
	case SIGINT:
		iint = 1;
		break;
	case SIGSEGV:
		segv = 1;
		break;
	case SIGTERM:
		term = 1;
		break;
	default:
		def = 1;
		break;
	}
}

void
handler2(int sig)
{
	switch (sig) {
	case SIGABRT:
		abrt = 1;
		break;
	case SIGFPE:
		fpe = 1;
		break;
	case SIGINT:
		iint = 1;
		break;
	case SIGSEGV:
		segv = 1;
		break;
	case SIGTERM:
		term = 1;
		break;
	default:
		def = 1;
		break;
	}
	longjmp(recover, 1);
}

void
test1()
{
	puts("test 1");
	assert(signal(SIGABRT, handler1) == SIG_DFL);
	assert(signal(SIGFPE, handler1) == SIG_DFL);
	assert(signal(SIGINT, handler1) == SIG_DFL);
	assert(signal(SIGSEGV, handler1) == SIG_DFL);
	assert(signal(SIGTERM, handler1) == SIG_DFL);
}

void
test2()
{
	puts("test 2");
	term = abrt = fpe = iint = segv = 0;
	assert(signal(SIGABRT, handler1) == handler1);
	assert(signal(SIGFPE, handler1) == handler1);
	assert(signal(SIGINT, handler1) == handler1);
	assert(signal(SIGSEGV, handler1) == handler1);
	assert(signal(SIGTERM, handler1) == handler1);

	assert(raise(SIGABRT) != -1);
	assert(raise(SIGFPE) != -1);
	assert(raise(SIGINT) != -1);
	assert(raise(SIGSEGV) != -1);
	assert(raise(SIGTERM) != -1);

	if (!abrt || !fpe || !iint || !segv || !term)
		printf("a handled signal was missed: %d %d %d %d %d\n",
		       abrt, fpe, iint, segv, term);
	if (def)
		puts("a wrong signal was received");
}

void
test3()
{
	puts("test 3");
	def = abrt = term = fpe = iint = segv = 0;
	assert(signal(SIGABRT, SIG_IGN) == handler1);
	assert(signal(SIGFPE, SIG_IGN) == handler1);
	assert(signal(SIGINT, SIG_IGN) == handler1);
	assert(signal(SIGSEGV, SIG_IGN) == handler1);
	assert(signal(SIGTERM, SIG_IGN) == handler1);

	assert(raise(SIGABRT) != -1);
	assert(raise(SIGFPE) != -1);
	assert(raise(SIGINT) != -1);
	assert(raise(SIGSEGV) != -1);
	assert(raise(SIGTERM) != -1);

	if (abrt || fpe || iint || segv || term)
		printf("a handled signal was received: %d %d %d %d %d\n",
		       abrt, fpe, iint, segv, term);
	if (def)
		puts("a wrong signal was received");
}

void
test4()
{
	puts("test 4");
	def = abrt = term = fpe = iint = segv = 0;
	assert(signal(SIGABRT, handler2) == SIG_IGN);
	assert(signal(SIGFPE, handler2) == SIG_IGN);
	assert(signal(SIGINT, handler2) == SIG_IGN);
	assert(signal(SIGSEGV, handler2) == SIG_IGN);
	assert(signal(SIGTERM, handler2) == SIG_IGN);

	if (!setjmp(recover))
		assert(raise(SIGABRT) != -1);
	if (!setjmp(recover))
		assert(raise(SIGFPE) != -1);
	if (!setjmp(recover))
		assert(raise(SIGINT) != -1);
	if (!setjmp(recover))
		assert(raise(SIGSEGV) != -1);
	if (!setjmp(recover))
		assert(raise(SIGTERM) != -1);

	if (!abrt || !fpe || !iint || !segv || !term)
		printf("a handled signal was missed: %d %d %d %d %d\n",
		       abrt, fpe, iint, segv, term);
	if (def)
		puts("a wrong signal was received");
}

int
main()
{
	assert(SIG_ERR != SIG_IGN && SIG_ERR != SIG_DFL);

	test1();
	test2();
	test3();
	test4();

	return;
}
