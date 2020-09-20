/* Taken from plan9 kernel */

typedef struct Clock0link Clock0link;
typedef struct Clock0link {
	int             (*clock)(void);
	Clock0link*     link;
} Clock0link;


int
f(void)
{
	return 0;
}

Clock0link cl0 = {
	.clock = f;
};

int
main(void)
{
	return (*cl0.clock)();
}
