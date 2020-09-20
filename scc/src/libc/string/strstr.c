#include <stddef.h>
#include <string.h>
#undef strstr

char *
strstr(const char *s1, const char *s2)
{
	const char *p;
	int c = *s2;

	if (c == '\0')
		return NULL;
	for (p = s1; p = strchr(p, c); ++p) {
		if (!strcmp(p, s2))
			return (char *) p;
	}
	return NULL;
}
