#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <scc/mach.h>

#include "libmach.h"

Map *
newmap(int n, FILE *fp)
{
	size_t vsiz;
	struct mapsec *p;
	Map *map;

	if (n > SIZE_MAX/sizeof(struct mapsec))
		goto out_range;
	vsiz = n * sizeof(struct mapsec);
	if (vsiz > SIZE_MAX - sizeof(*map))
		goto out_range;
	vsiz += sizeof(*map);

	if ((map = malloc(vsiz)) == NULL)
		return NULL;

	map->n = n;
	memset(map->sec, 0, vsiz);

	for (p = map->sec; n--; ++p)
		p->fp = fp;
	return map;

out_range:
	errno = ERANGE;
	return NULL;
}
