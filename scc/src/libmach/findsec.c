#include <stdio.h>

#include <scc/mach.h>

#include "libmach.h"

int
findsec(Map *map, char *name)
{
	int i;
	struct mapsec *sec;

	for (i = 0; i < map->sec; i++) {
		char *s = map->sec[i].name;
		if (s && strcmp(s, name) == 0)
			return i;
	}

	return -1;
}
