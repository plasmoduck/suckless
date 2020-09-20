/* This file is inspired in the book "Understanding and using COFF" */

struct aouthdr {
	short magic;        /* magic number */
	short vstamp;       /* version stamp */
	long tsize;         /* text size in bytes */
	long dsize;         /* initialized data size */
	long bsize;         /* uinitialized data size */
	long entry;         /* entry point */
	long text_start;    /* base of text segment */
	long data_start;    /* base of data segment */
};

#define AOUTHDR struct aouthdr 
#define AOUTSZ sizeof(AOUTHDR)

#define QMAGIC      0314
#define STMAGIC     0401
#define OMAGIC      0404
#define JMAGIC      0407
#define DMAGIC      0410
#define ZMAGIC      0413
