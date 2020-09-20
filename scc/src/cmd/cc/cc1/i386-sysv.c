#include <scc/scc.h>
#include "cc1.h"

#define RANK_BOOL    0
#define RANK_SCHAR   1
#define RANK_UCHAR   1
#define RANK_CHAR    1
#define RANK_SHORT   2
#define RANK_USHORT  2
#define RANK_INT     3
#define RANK_UINT    3
#define RANK_LONG    4
#define RANK_ULONG   4
#define RANK_LLONG   5
#define RANK_ULLONG  5
#define RANK_FLOAT   6
#define RANK_DOUBLE  7
#define RANK_LDOUBLE 8

static int
local_valid_va_list(Type *tp)
{
	return eqtype(tp, va_list_type, 1);
}

Arch *
i386_sysv(void)
{
	static Arch arch = {
		.voidtype = {
			.op = VOID,
			.letter = L_VOID,
		},
		.pvoidtype = {
			.op = PTR,
			.letter = L_POINTER,
			.prop = TDEFINED,
			.size = 8,
			.align = 8,
		},
		.booltype = {
			.op = INT,
			.letter = L_BOOL,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 1,
			.align = 1,
			.n.rank = RANK_BOOL,
		},
		.schartype = {
			.op = INT,
			.letter = L_INT8,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 1,
			.align = 1,
			.n.rank = RANK_SCHAR,

		},
		.uchartype = {
			.op = INT,
			.letter = L_UINT8,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 1,
			.align = 1,
			.n.rank = RANK_UCHAR,
		},
		.chartype = {
			.op = INT,
			.letter = L_INT8,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 1,
			.align = 1,
			.n.rank = RANK_CHAR,
		},
		.ushortype = {
			.op = INT,
			.letter = L_UINT16,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 2,
			.align = 2,
			.n.rank = RANK_USHORT,

		},
		.shortype = {
			.op = INT,
			.letter = L_INT16,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 2,
			.align = 2,
			.n.rank = RANK_SHORT,
		},
		.uinttype = (Type) {
			.op = INT,
			.letter = L_UINT32,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 4,
			.align = 4,
			.n.rank = RANK_UINT,
		},
		.inttype = (Type) {
			.op = INT,
			.letter = L_INT32,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 4,
			.align = 4,
			.n.rank = RANK_INT,
		},
		.longtype = {
			.op = INT,
			.letter = L_INT32,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 4,
			.align = 4,
			.n.rank = RANK_LONG,
		},
		.ulongtype = {
			.op = INT,
			.letter = L_UINT64,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 8,
			.align = 4,
			.n.rank = RANK_ULLONG,
		},
		.ullongtype = {
			.op = INT,
			.letter = L_UINT64,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 8,
			.align = 8,
			.n.rank = RANK_ULLONG,
		},
		.llongtype = {
			.op = INT,
			.letter = L_INT64,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 8,
			.align = 8,
			.n.rank = RANK_LLONG,
		},
		.floattype = {
			.op = FLOAT,
			.letter = L_FLOAT,
			.prop = TDEFINED | TARITH,
			.size = 4,
			.align = 4,
			.n.rank = RANK_FLOAT,
		},
		.doubletype = {
			.op = FLOAT,
			.letter = L_DOUBLE,
			.prop = TDEFINED | TARITH,
			.size = 8,
			.align = 4,
			.n.rank = RANK_DOUBLE,
		},
		.ldoubletype = {
			.op = FLOAT,
			.letter = L_LDOUBLE,
			.prop = TDEFINED | TARITH,
			.size = 12,
			.align = 4,
			.n.rank = RANK_LDOUBLE,
		},
		.sizettype = {
			.op = INT,
			.letter = L_UINT32,
			.prop = TDEFINED | TINTEGER | TARITH,
			.size = 4,
			.align = 4,
			.n.rank = RANK_UINT,
		},
		.pdifftype = {
			.op = INT,
			.letter = L_INT32,
			.prop = TDEFINED | TINTEGER | TARITH | TSIGNED,
			.size = 4,
			.align = 4,
			.n.rank = RANK_INT,
		},
		.ellipsistype = {
			.op = ELLIPSIS,
			.letter = L_ELLIPSIS,
			.prop = TDEFINED,
		},
		.zero = {
			.u.i = 0,
		},
		.one = {
			.u.i = 1,
		},
		.va_type = {
			.op = PTR,
			.letter = L_POINTER,
			.prop = TDEFINED,
			.size = 4,
			.align = 4,
		},
		.va_list_type = {
			.op = PTR,
			.letter = L_POINTER,
			.prop = TDEFINED,
			.size = 4,
			.align = 4,
		},
	};

	arch.valid_va_list = local_valid_va_list;
	arch.pvoidtype.type = &arch.chartype;
	arch.va_list_type.type = &arch.longtype;

	return &arch;
}
