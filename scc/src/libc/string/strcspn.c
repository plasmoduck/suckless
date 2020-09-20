#include <string.h>
#undef strcspn

size_t
strcspn(const char *s1, const char *s2)
{
	const unsigned char *s = s1;
	const unsigned char *accept = s2;
	unsigned ch;
	size_t n;
	char buf[__NUMCHARS];

	memset(buf, 0, sizeof(buf));
	while (ch = *accept++)
		buf[ch] = 1;

	for (n = 0; (ch = *s++) && !buf[ch]; ++n)
		;

	return n;
}
