#ifdef _NEED_SIZET
#ifndef _SIZET
typedef unsigned short size_t;
#define _SIZET
#endif
#undef _NEED_SIZET
#endif

#ifdef _NEED_PTRDIFFT
#ifndef _PTRDIFFT
typedef short ptrdiff_t;
#define _PTRDIFFT
#endif
#undef _NEED_PTRDIFFT
#endif

#ifdef _NEED_NULL
#ifndef NULL
#define NULL ((void *) 0)
#endif
#undef _NEED_NULL
#endif

#ifdef _NEED_WCHART
#ifndef _WCHART
typedef int wchar_t;
#define _WCHART
#endif
#undef _NEED_WCHART
#endif
