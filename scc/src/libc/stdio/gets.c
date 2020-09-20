#include <stdio.h>
#undef gets

char *
gets(char *s)
{
	int ch;
	char *t = s;

	while ((ch = getc(stdin)) != EOF && ch != '\n')
		*t++ = ch;
	if (ch == EOF && s == t)
		return NULL;
	*t = '\0';

	return s;
}
