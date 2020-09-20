typedef struct segment Segment;
typedef struct section Section;
typedef struct symbol Symbol;
typedef struct objops Objops;
typedef struct obj Obj;
typedef struct map Map;
typedef struct mapsec Mapsec;

enum sectype {
	SREAD   = 1 << 0,
	SWRITE  = 1 << 1,
	SEXEC   = 1 << 2,
	SLOAD   = 1 << 3,
	SALLOC  = 1 << 4,
	SRELOC  = 1 << 5,
	SABS    = 1 << 6,
	SSHARED = 1 << 7,
};

struct obj {
	char *index;
	Objops *ops;
	int type;
	long pos;
	void *data;
	Obj *next;
};

struct segment {
	char *name;
	unsigned long long base;
	unsigned long long size;
	unsigned flags;
	int index;
	int type;
	int align;
	int nsec;
	Section **sections;
};

struct section {
	char *name;
	unsigned long long base;
	unsigned long long size;
	unsigned flags;
	int index;
	int type;
	int align;
};

struct symbol {
	char *name;
	unsigned long long size;
	unsigned long long value;
	int index;
	int section;
	char type;
};

extern int archive(FILE *fp);
extern long armember(FILE *fp, char *member);

extern int objtype(FILE *fp, char **name);
extern Obj *newobj(int type);
extern void delobj(Obj *obj);

extern int readobj(Obj *obj, FILE *fp);
extern int writeobj(Obj *obj, Map *map, FILE *fp);

extern int strip(Obj *obj);
extern int pc2line(Obj *obj, unsigned long long pc, char *fname, int *ln);
extern int rebase(Obj *obj, int index, unsigned long long offset);

extern Map *loadmap(Obj *obj, FILE *fp);
extern Map *newmap(int n, FILE *fp);
extern int findsec(Map *map, char *name);
extern int setmap(Map *map,
                  char *name,
                  FILE *fp,
                  unsigned long long begin,
                  unsigned long long end,
                  long off);

extern Symbol *getsym(Obj *obj, int *index, Symbol *sym);
extern Section *getsec(Obj *obj, int *index, Section *sec);

extern int setindex(int, long, char **, long *, FILE *);
extern int getindex(int, long *, char ***, long **, FILE *);
