#define SUN      0
#define MON      1
#define TUE      2
#define WED      3
#define THU      4
#define FRI      5
#define SAT      6

#define JAN      0
#define FEB      1
#define DEC     11

#define EPOCH 1970
#define FEBDAYS(y) ((_daysyear(y) == 366) ? 29 : 28)
#define SECMIN     60
#define SECHOUR    (60 * SECMIN)    /* 3600 */
#define SECDAY     (24 * SECHOUR)   /* 86400 */

struct tzone {
	char *name;
	int gmtoff;
	int isdst;
};

struct tm;

extern struct tzone *_tzone(struct tm *tm);
extern int _daysyear(int year);
extern int _newyear(int year);
extern void *_getheap(void);
extern int _dtoi(char);

#ifdef stdin
extern int _flsbuf(FILE *fp);
extern int _allocbuf(FILE *fp);
extern FILE *_fpopen(const char * restrict fname,
                     const char * restrict mode,
                     FILE * restrict fp);
#endif

extern int _daysmon[12];
