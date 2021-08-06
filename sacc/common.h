#define clear(p)	do { void **_p = (void **)(p); free(*_p); *_p = NULL; } while (0);

#define CYAN   "\x1b[36m"
#define YELLOW "\x1b[33m"
#define BOLD   "\x1b[1m"
#define BLUE   "\x1b[34m"
#define RED    "\x1b[31m"
#define RESET  "\x1b[0m"

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
