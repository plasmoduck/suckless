enum args {
	AREG_AF = AMAX,
	AREG_A,
	AREG_F,

	AREG_BC,
	AREG_B,
	AREG_C,

	AREG_HL,
	AREG_H,
	AREG_L,

	AREG_DE,
	AREG_D,
	AREG_E,

	AREG_IX,
	AREG_IXL,
	AREG_IXH,

	AREG_IY,
	AREG_IYL,
	AREG_IYH,

	AREG_R,
	AREG_I,
	AREG_AF_,
	AREG_SP,

	AREG_NZ,
	AREG_Z,
	AREG_NC,
	AREG_PO,
	AREG_PE,
	AREG_P,
	AREG_M,

	AREG_RCLASS,  /* register class for B, C, D, E, H, L and A */
	AREG_PCLASS,  /* register class for B, C, D, E, IXH, IXL and A */
	AREG_QCLASS,  /* register class for B, C, D, E, IYH, IYL and A */
	AREG_DDCLASS, /* register class for BC, DE, HL and SP */
	AREG_QQCLASS, /* register class for BC, DE, HL and AF */
	AREG_PPCLASS, /* register class for BC, DE, IX and SP */
	AREG_RRCLASS, /* register class for BC, DE, IY and SP */
	AREG_SSCLASS, /* flag class for C, NC, Z, NZ */
	AREG_CCCLASS, /* flag class for NZ, Z, NC, C, PO, PE, P, M */

	AINDEX_IX,    /* (IX+d) */
	AINDEX_IY,    /* (IX+d) */

	AINDER_HL,    /* (HL) */
	AINDER_DE,    /* (DE) */
	AINDER_BC,    /* (BC) */
	AINDER_SP,    /* (SP) */
	AINDER_C,     /* (C) */
	AINDER_IX,    /* (IX) */
	AINDER_IY,    /* (IY) */

	AZERO,         /* a literal 0 */
	ARST,          /* 0h, 08h, 10h, 18h, 20h, 28h, 30h, 38h */
};

enum class {
	RCLASS  = 1 << 0,
	PCLASS  = 1 << 1,
	QCLASS  = 1 << 2,
	DDCLASS = 1 << 3,
	QQCLASS = 1 << 4,
	PPCLASS = 1 << 5,
	RRCLASS = 1 << 6,
	CCCLASS = 1 << 7,
	SSCLASS = 1 << 8,
};
