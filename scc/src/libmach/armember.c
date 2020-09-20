#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/ar.h>
#include <scc/mach.h>

static char *
getfname(struct ar_hdr *hdr, char *dst)
{
        char *p;
        int i;

        memcpy(dst, hdr->ar_name, SARNAM);
        dst[SARNAM] = '\0';

        for (i = SARNAM-1; i > 0; i--) {
                if (dst[i] != ' ' && dst[i] != '/')
                        break;
                dst[i] = '\0';
        }
        return dst;
}

long
armember(FILE *fp, char *member)
{
	struct ar_hdr hdr;
	long siz;

	if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
		return (feof(fp)) ? 0 : -1;

	if (strncmp(hdr.ar_fmag, ARFMAG, sizeof(hdr.ar_fmag))) {
		errno = ERANGE;
		return -1;
	}

	siz = strtol(hdr.ar_size, NULL, 0);
	if (siz & 1)
		siz++;
	if (siz == 0) {
		errno = ERANGE;
		return -1;
	}

	getfname(&hdr, member);

	return sizeof(hdr) + siz;
}
