#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/mach.h>
#include <scc/cstd.h>

#include "../libmach.h"
#include "coff32.h"

int
coff32xgetidx(int order, long *nsyms, char ***namep, long **offsp, FILE *fp)
{
	long i, n;
	long *offs;
	char **names;
	unsigned char buf[EXTIDENTSIZ+1];

	if (fread(buf, 4, 1, fp) != 1)
		return -1;
	unpack(order, buf, "l", &n);

	if (n <= 0)
		return -1;

	if ((names = calloc(sizeof(char *), n)) == NULL)
		return -1;

	if ((offs = calloc(sizeof(long), n)) == NULL)
		goto err1;

	for (i = 0; i < n; i++) {
		fread(buf, 4, 1, fp);
		unpack(order, buf, "l", offs[i]);
	}

	for (i = 0; i < n; i++) {
		int j, c;
		char *s;

		for (j = 0; j < EXTIDENTSIZ; j++) {
			if ((c = getc(fp)) == EOF || c == '\0')
				break;
			buf[j] = c;
		}
		if (c != '\0')
			goto err2;
		buf[j] = '\0';

		if ((s = malloc(j)) == NULL)
			goto err2;
		memcpy(s, buf, j);
		names[i]= s;
	}

	if (ferror(fp))
		goto err2;

	*offsp = offs;
	*namep = names;
	*nsyms = n;

	return 0;

err2:
	free(offs);
err1:
	for (i = 0; i < n; i++)
		free(names[i]);
	free(*names);
	return -1;
}
