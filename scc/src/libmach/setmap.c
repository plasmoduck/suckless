#include <stdio.h>
#include <string.h>

#include <scc/mach.h>

#include "libmach.h"

int
setmap(Map *map,
       char *name,
       FILE *fp,
       unsigned long long begin,
       unsigned long long end,
       long off)
{
	int n;
	Mapsec *sec;

	n = map->n;
	for (sec = map->sec; n--; sec++) {
		if (!sec->name) {
			sec->name = name;
			sec->fp = fp,
			sec->begin = begin;
			sec->end = end;
			sec->offset = off;
			return 0;
		}
	}

	return -1;
}