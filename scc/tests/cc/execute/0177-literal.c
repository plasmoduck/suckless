void boo(int *p)
{
	return (*p[1] == 2) ? 0 : 1;
}

int main()
{
	return boo((int[]) {0, 2});
}
