#include <scc/coff32/filehdr.h>
#include <scc/coff32/aouthdr.h>
#include <scc/coff32/scnhdr.h>
#include <scc/coff32/syms.h>
#include <scc/coff32/reloc.h>
#include <scc/coff32/linenum.h>

typedef struct coff32 Coff32;

struct arch {
	char *name;
	unsigned char magic[2];
	int type;
};

struct coff32 {
	FILHDR hdr;
	AOUTHDR aout;
	SCNHDR *scns;
	SYMENT *ents;
	RELOC **rels;
	LINENO **lines;
	char *strtbl;
	unsigned long strsiz;
};

extern int coff32new(Obj *obj);
extern int coff32read(Obj *obj, FILE *fp);
extern int coff32setidx(long nsyms, char **names, long *offs, FILE *fp);
extern int coff32getidx(long *nsyms, char ***namep, long **offsp, FILE *fp);
extern int coff32pc2line(Obj *, unsigned long long , char *, int *);
extern int coff32strip(Obj *obj);
extern void coff32del(Obj *obj);
extern int coff32write(Obj *obj, Map * map, FILE *fp);
extern int coff32probe(unsigned char *buf, char **name);

extern int coff32xsetidx(int order,
                         long nsymbols, char *names[], long offs[], FILE *fp);
extern int coff32xgetidx(int order,
                         long *nsyms, char ***namep, long **offsp, FILE *fp);

extern Symbol *coff32getsym(Obj *obj, int *idx, Symbol *sym);
extern Section *coff32getsec(Obj *obj, int *idx, Section *sec);
extern Map *coff32loadmap(Obj *obj, FILE *fp);
