#define clear(p)	do { void **_p = (void **)(p); free(*_p); *_p = NULL; } while (0);

typedef struct item Item;
typedef struct dir Dir;

struct item {
	char type;
	char redtype;
	char *username;
	char *selector;
	char *host;
	char *port;
	char *raw;
	char *tag;
	void *dat;
	Item *entry;
};

struct dir {
	Item *items;
	size_t nitems;
	size_t printoff;
	size_t curline;
};

void die(const char *fmt, ...);
size_t mbsprint(const char *s, size_t len);
#ifdef NEED_STRCASESTR
char *strcasestr(const char *h, const char *n);
#endif /* NEED_STRCASESTR */
const char *typedisplay(char t);
void uicleanup(void);
void uidisplay(Item *entry);
char *uiprompt(char *fmt, ...);
Item *uiselectitem(Item *entry);
void uisetup(void);
void uisigwinch(int signal);
void uistatus(char *fmt, ...);
