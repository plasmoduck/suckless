#include <ctype.h>
#include <scc/scc.h>

int
casecmp(const char *s1, const char *s2)
{
        while (*s1 && toupper(*s1) == toupper(*s2))
                ++s1, ++s2;
        return *s1 - *s2;
}
