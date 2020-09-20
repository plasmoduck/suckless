/*
 * First 3 bits of flags in segments and symbols are for the
 * type of segment
 */
enum symflags {
	FREG    = 1 << 0,
	FSECT   = 1 << 1,
	FSYM    = 1 << 2,
	FCOMMON = 1 << 3,
	FEXTERN = 1 << 4,
	FDEF    = 1 << 5,
	FGLOBAL = 1 << 6,
	FABS    = 1 << 7,
};

enum secflags {
	SREAD  = 1 << 0,
	SWRITE = 1 << 1,
	SEXEC  = 1 << 2,
	SLOAD  = 1 << 3,
	SFILE  = 1 << 4,
	SABS   = 1 << 5,
};

enum endianess {
	BIG_ENDIAN    = -1,
	LITTLE_ENDIAN = 1
};

enum common_args {
	AIMM = 1,
	ASTR,
	AREG,
	ANUMBER,
	AIMM2,
	AIMM3,
	AIMM5,
	AIMM8,
	AIMM16,
	AIMM32,
	AIMM64,
	AINDIR,
	AINDEX,
	ADIRECT,
	AREG_OFF,
	ASYM,
	AOPT,
	AREP,
	AMAX,
};

enum tokens {
	EOS = -1,
	IDEN = 1,
	NUMBER,
	REG,
	STRING,
	MINUS,
	SHL,
	SHR,
	GE,
	LE,
};

#define MAXSYM 63

typedef struct reloc Reloc;
typedef struct ins Ins;
typedef struct op Op;
typedef struct section Section;
typedef struct symbol Symbol;
typedef struct node Node;
typedef struct string String;
typedef void Format(Op *, Node **);

struct string {
	char *buf;
	size_t offset;
};

struct line {
	char *label;
	char *op;
	char *args;
};

struct ins {
	int begin, end;
	char *str;
};

struct reloc {
	size_t offset;
	Symbol *sym;
	unsigned char flags;
	unsigned char size;
	unsigned char nbits;
	unsigned char shift;
};

struct op {
	unsigned char flags;
	unsigned char size;
	void (*format)(Op *, Node **);
	unsigned char *bytes;
	unsigned char *args;
};

struct section {
	Symbol *sym;
	char *mem;
	unsigned char flags;
	unsigned char fill;
	unsigned char aligment;
	unsigned id;
	TUINT base;
	TUINT max;
	TUINT curpc;
	TUINT pc;
	struct section *next;
};

struct symbol {
	String name;
	String type;
	unsigned char flags;
	unsigned char pass;
	TUINT value;
	TUINT size;
	Section *section;
	struct symbol *next;
	struct symbol *hash;
};

struct node {
	unsigned char op;
	unsigned char addr;
	struct symbol *sym;
	struct node *left;
	struct node *right;
};

union yylval {
	TUINT val;
	Symbol *sym;
};


/* symbol.c */
extern void cleansecs(void);
extern void isecs(void);
extern void emit(char *bytes, int nbytes);
extern Section *setsec(char *name, char *attr);
extern Symbol *tmpsym(TUINT val);
extern void killtmp(void);
extern int toobig(Node *np, int type);
extern void dumpstab(char *msg);
extern String newstring(char *s);

/* main.c */
extern Symbol *lookup(char *name);
extern Symbol *deflabel(char *name);

/* parser.c */
extern Node **getargs(char *s);
extern void error(char *msg, ...);
/* Avoid errors in files where stdio is not included */
#ifdef stdin
extern int nextline(struct line *linep);
#endif
extern void unexpected(void);
extern void expect(int token);
int next(void);
#define accept(t) (yytoken == (t) ? next() : 0)
extern void regctx(int mode);
extern Node *getreg(void);
extern Node *operand(char **s);
extern void addinput(char *fname);
extern int delinput(void);

/* expr.c */
extern Node *expr(void);
extern void deltree(Node *np);
extern Node *node(int op, Node *l, Node *r);

/* proc.c */
extern void iarch(void);
extern int match(Op *op, Node **args);
extern Node *moperand(void);

/* ins.c */
extern char *tobytes(TUINT v, int n, int inc);

/*
 * Definition of global variables
 */
extern Section *cursec, *seclist;
extern int nr_ins;
extern Ins instab[];
extern Op optab[];
extern int pass;
extern TUINT maxaddr;
extern int endian;
extern Symbol *linesym, *symlist;
extern char *infile;
extern int endpass;
extern int yytoken;
extern size_t yylen;
extern union yylval yylval;
extern char yytext[];
