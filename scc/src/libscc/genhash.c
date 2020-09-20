unsigned
genhash(char *name)
{
	unsigned h;
	char c;

	for (h = 0; c = *name; ++name)
		h = h*33 ^ c;

	return h;
}
