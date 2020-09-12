#include <sys/types.h>

#include <stdio.h>

#ifdef __OpenBSD__
#include <unistd.h>
#else
#define pledge(p1,p2) 0
#endif

#undef strlcat
size_t strlcat(char *, const char *, size_t);
#undef strlcpy
size_t strlcpy(char *, const char *, size_t);

/* feed info */
struct feed {
	char         *name;     /* feed name */
	unsigned long totalnew; /* amount of new items per feed */
	unsigned long total;    /* total items */
};

/* uri */
struct uri {
	char proto[48];
	char host[256];
	char path[2048];
	char port[6];     /* numeric port */
};

enum {
	FieldUnixTimestamp = 0, FieldTitle, FieldLink, FieldContent,
	FieldContentType, FieldId, FieldAuthor, FieldEnclosure, FieldLast
};

int  absuri(char *, size_t, const char *, const char *);
void parseline(char *, char *[FieldLast]);
int  parseuri(const char *, struct uri *, int);
void printutf8pad(FILE *, const char *, size_t, int);
int  strtotime(const char *, time_t *);
void xmlencode(const char *, FILE *);
