#define NBYTES 20
#define OBJ(format,arch,order) ((order) << 10 | (arch) << 5 | (format)) 
#define FORMAT(t) ((t) & 0x1f)
#define ARCH(t) (((t) >> 5) & 0x1f)
#define ORDER(t) (((t) >> 10) & 0x1f)


enum objformat {
	COFF32,
	NFORMATS,
};

enum objarch {
	ARCH286,
	ARCH386,
	ARCHAMD64,
	ARCHZ80,
	ARCHARM32,
	ARCHARM64,
};

enum order {
	LITTLE_ENDIAN,
	BIG_ENDIAN,
};

struct objops {
	int (*probe)(unsigned char *buf, char **name);

	int (*new)(Obj *obj);
	void (*del)(Obj *obj);

	int (*read)(Obj *obj, FILE *fp);
	int (*write)(Obj *obj, Map *map, FILE *fp);

	int (*strip)(Obj *obj);
	int (*pc2line)(Obj *, unsigned long long , char *, int *);

	Map *(*loadmap)(Obj *obj, FILE *fp);

	Symbol *(*getsym)(Obj *obj, int *index, Symbol *sym);
	Section *(*getsec)(Obj *obj, int *index, Section *sec);

	int (*setidx)(long nsyms, char *names[], long offset[], FILE *fp);
	int (*getidx)(long *nsyms, char ***names, long **offset, FILE *fp);
};


struct map {
	int n;
	struct mapsec {
		char *name;
		FILE *fp;
		unsigned long long begin;
		unsigned long long end;
		long offset;
	} sec[];
};

/* common functions */
extern int pack(int order, unsigned char *dst, char *fmt, ...);
extern int unpack(int order, unsigned char *src, char *fmt, ...);
extern int objpos(Obj *obj, FILE *fp, long pos);

/* globals */
extern Objops *objops[];
extern Objops coff32;
