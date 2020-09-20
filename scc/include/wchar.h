#ifndef _WCHAR_H
#define _WCHAR_H

/* TODO: This is only a placeholder */
typedef long wchar_t;
/* typedef long size_t; */
typedef long mbstate_t;
typedef long wint_t;
struct tm;

/* #define WCHAR_MAX 1 */
/* #define WCHAR_MIN 1 */
/* #define WEOF -1 */
/* #define NULL 0 */

#ifdef _STDARG_H
extern int vswscanf(const wchar_t * restrict s, const wchar_t * restrict format, va_list arg);
extern int vwprintf(const wchar_t * restrict format, va_list arg);
extern int vwscanf(const wchar_t * restrict format, va_list arg);
#endif

#ifdef _STDIO_H
extern int fwprintf(FILE * restrict stream, const wchar_t * restrict format, ...);
extern int fwscanf(FILE * restrict stream, const wchar_t * restrict format, ...);

#ifdef _STDARG_H
extern int vfwprintf(FILE * restrict stream, const wchar_t * restrict format, va_list arg);
extern int vfwscanf(FILE * restrict stream, const wchar_t * restrict format, va_list arg);
extern int vswprintf(wchar_t * restrict s, size_t n, const wchar_t * restrict format, va_list arg);
#endif

extern wint_t fgetwc(FILE *stream);
extern wint_t fputwc(wchar_t c, FILE *stream);
extern wint_t getwc(FILE *stream);
extern wint_t putwc(wchar_t c, FILE *stream);
extern int fwide(FILE *stream, int mode);
extern wint_t ungetwc(wint_t c, FILE *stream);
extern wchar_t *fgetws(wchar_t * restrict s, int n, FILE * restrict stream);
extern int fputws(const wchar_t * restrict s, FILE * restrict stream);
#endif

extern int swprintf(wchar_t * restrict s, size_t n, const wchar_t * restrict format, ...);
extern int swscanf(const wchar_t * restrict s, const wchar_t * restrict format, ...);
extern int wprintf(const wchar_t * restrict format, ...);
extern int wscanf(const wchar_t * restrict format, ...);

extern wint_t getwchar(void);
extern wint_t putwchar(wchar_t c);

extern double wcstod(const wchar_t * restrict nptr, wchar_t ** restrict endptr);
extern float wcstof(const wchar_t * restrict nptr, wchar_t ** restrict endptr);
extern long double wcstold(const wchar_t * restrict nptr, wchar_t ** restrict endptr);

extern long int wcstol(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
extern long long int wcstoll(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
extern unsigned long int wcstoul(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);
extern unsigned long long int wcstoull(const wchar_t * restrict nptr, wchar_t ** restrict endptr, int base);

extern wchar_t *wcscpy(wchar_t * restrict s1, const wchar_t * restrict s2);
extern wchar_t *wcsncpy(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);

extern wchar_t *wmemcpy(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
extern wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
extern wchar_t *wcscat(wchar_t * restrict s1, const wchar_t * restrict s2);
extern wchar_t *wcsncat(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
extern int wcscmp(const wchar_t *s1, const wchar_t *s2);
extern int wcscoll(const wchar_t *s1, const wchar_t *s2);
extern int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
extern size_t wcsxfrm(wchar_t * restrict s1, const wchar_t * restrict s2, size_t n);
extern int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n);
extern wchar_t *wcschr(const wchar_t *s, wchar_t c);
extern size_t wcscspn(const wchar_t *s1, const wchar_t *s2);
extern wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2);
extern wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
extern size_t wcsspn(const wchar_t *s1, const wchar_t *s2);
extern wchar_t *wcsstr(const wchar_t *s1, const wchar_t *s2);
extern wchar_t *wcstok(wchar_t * restrict s1, const wchar_t * restrict s2, wchar_t ** restrict ptr);
extern wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
extern size_t wcslen(const wchar_t *s);
extern wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);
extern size_t wcsftime(wchar_t * restrict s, size_t maxsize, const wchar_t * restrict format, const struct tm * restrict timeptr);
extern wint_t btowc(int c);
extern int wctob(wint_t c);
extern int mbsinit(const mbstate_t *ps);
extern size_t mbrlen(const char * restrict s, size_t n, mbstate_t * restrict ps);
extern size_t mbrtowc(wchar_t * restrict pwc, const char * restrict s, size_t n, mbstate_t * restrict ps);
extern size_t wcrtomb(char * restrict s, wchar_t wc, mbstate_t * restrict ps);
extern size_t mbsrtowcs(wchar_t * restrict dst, const char ** restrict src, size_t len, mbstate_t * restrict ps);
extern size_t wcsrtombs(char * restrict dst, const wchar_t ** restrict src, size_t len, mbstate_t * restrict ps);


#endif
