/* This file is inspired in the book "Understanding and using COFF" */

struct reloc {
	long r_vaddr;           /* address of reference */
	long r_symndx;          /* index into symbol table */
	unsigned short r_type; /* relocation type */
};

#define RELOC           struct reloc
#define RELSZ           10              /* sizeof (RELOC) */
