extern void __assert(char *exp, char *file, long line);

#undef assert
#ifndef NDEBUG
# define assert(exp) ((exp) ? (void) 0 : __assert(#exp, __FILE__, __LINE__))
#else
# define assert(exp) ((void)0)
#endif
