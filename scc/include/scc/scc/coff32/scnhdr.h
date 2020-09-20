/* This file is inspired in the book "Understanding and using COFF" */

#define SCNNMLEN 8

struct scnhdr {
	char s_name[SCNNMLEN];    /* section name */
	long s_paddr;             /* physical address */
	long s_vaddr;             /* virtual address */
	long s_size;              /* section size */
	long s_scnptr;            /* file ptr to raw data */
	long s_relptr;            /* file ptr to relo info */
	long s_lnnoptr;           /* file ptr to line number */
	unsigned short s_nrelloc; /* number of relocation entries */
	unsigned short s_nlnno;   /* number of lines entries */
	long s_flags;             /* type and content flags */
};

#define SCNHDR struct scnhdr
#define SCNHSZ 40

#define STYP_REG         0
#define STYP_DSECT       (1 << 0)
#define STYP_NOLOAD      (1 << 1)
#define STYP_GROUP       (1 << 2)
#define STYP_PAD         (1 << 3)
#define STYP_COPY        (1 << 4)
#define STYP_TEXT        (1 << 5)
#define STYP_DATA        (1 << 6)
#define STYP_BSS         (1 << 7)
#define STYP_INFO        (1 << 9)
#define STYP_OVER        (1 << 11)
#define STYP_LIB         (1 << 12)
