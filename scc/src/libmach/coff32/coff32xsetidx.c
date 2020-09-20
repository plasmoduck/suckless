#include <stdio.h>
#include <string.h>

#include <scc/mach.h>

#include "../libmach.h"

int
coff32xsetidx(int order, long nsyms, char *names[], long offs[], FILE *fp)
{
	long i, n;
	size_t len;
	unsigned char buff[4];

	pack(order, buff, "l", nsyms);
	fwrite(buff, 4, 1, fp);
	n = 4;

	for (i = 0; i < nsyms; i++) {
		pack(order, buff, "l", offs[i]);
		fwrite(buff, 4, 1, fp);
		n += 4;
	}

	for (i = 0; i < nsyms; i++) {
		len = strlen(names[i]) + 1;
		fwrite(names[i], len, 1, fp);
		n += len;
	}

	return fflush(fp) == EOF ? -1 : 0;
}
