struct fprop {
	unsigned uid;
	unsigned gid;
	unsigned long mode;
	long size;
	time_t time;
};

extern const char invalidchars[];

extern time_t totime(long long t);
extern char *canonical(char *path);
extern int getstat(char *fname, struct fprop *prop);
extern int setstat(char *fname, struct fprop *prop);
