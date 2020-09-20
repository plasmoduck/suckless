typedef struct List List;
struct List {
	int len;
	struct List *head;
	List *back;
};

int
main(void)
{
	List List;

	return List.len;
}
