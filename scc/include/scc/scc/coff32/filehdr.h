/* This file is inspired in the book "Understanding and using COFF" */

struct filehdr {
	unsigned short f_magic;  /* magic number */
	unsigned short f_nscns;  /* number of sections */
	long f_timdat;           /* time stamp */
	long f_symptr;           /* file pointer to symbols */
	long f_nsyms;            /* number of symbols */
	unsigned short f_opthdr; /* size of optional header */
	unsigned short f_flags;  /* file flags */
};

#define FILHDR struct filehdr
#define FILHSZ 20

#define F_RELFLG          0000001
#define F_EXEC            0000002
#define F_LMNO            0000004
#define F_SYMS            0000010
#define F_MINMAL          0000020
#define F_UPDATE          0000040
#define F_SWADB           0000100
#define F_AR16WR          0000200
#define F_AR32WR          0000400
#define F_A32WR           0001000
#define F_PATCH           0002000
#define F_NODF            0002000

#define COFF_I386MAGIC    0x014c
#define COFF_Z80MAGIC     0x805a
