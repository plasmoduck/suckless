/* This file is inspired in the book "Understanding and using COFF" */

struct lineno {
	long l_symndx;         /* index in table symbol if l_lnno == 0 */
	long l_paddr;          /* Break pointable address if l_lnno > 0 */
	unsigned short l_lnno; /* Line number */
};

#define LINENO struct lineno
#define LINESZ 6
