int
main(void)
{
	unsigned char uc;
	signed char sc;

	uc = -1;
	if ((uc & 0xFF) != 0xFF)
		return 1;

	uc = '\x23';
	if (uc != 36)
		return 1;

	uc = 1u;
	if (uc != (1025 & 0xFF)
		return 1;

	uc = 'A';
	if (uc != 0x41)
		return 1;

	sc = -1;
	if ((sc & 0xFF) != 0xFF)
		return 1;

	sc = '\x23';
	if (sc != 36)
		return 1;

	sc = 1u;
	if (uc != (1025 & 0xFF)
		return 1;

	sc = 'A';
	if (uc != 0x41)
		return 1;

	return 0;
}
