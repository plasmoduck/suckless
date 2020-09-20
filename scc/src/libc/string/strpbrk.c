#include <string.h>
#undef strpbrk

char *
strpbrk(const char *s1, const char *s2)
{
	const unsigned char *s = s1;
	const unsigned char *accept = s2;
	unsigned ch;
	char buf[__NUMCHARS];

	memset(buf, 0, sizeof(buf));
	while (ch = *accept++)
		buf[ch] = 1;

	while ((ch = *s) && !buf[ch])
		s++;

	return (ch == '\0') ? NULL : (char *) s;
}
