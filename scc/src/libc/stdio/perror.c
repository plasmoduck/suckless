#include <errno.h>
#include <stdio.h>
#include <string.h>
#undef perror

void
perror(const char *msg)
{
	if (msg && *msg) {
		fputs(msg, stderr);
		putc(':', stderr);
		putc(' ', stderr);
	}
	fputs(strerror(errno), stderr);
	putc('\n', stderr);
}
