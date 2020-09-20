#define O_RDONLY  0x00000000
#define O_WRONLY  0x00000001
#define O_RDWR    0x00000002

#define O_TRUNC   0x00000400
#define O_APPEND  0x00000008
#define O_CREAT   0x00000200

typedef int pid_t;

struct sigaction {
	void (*sa_handler)(int);
	char sa_mask[8];
	int sa_flags;
};

extern pid_t _getpid(void);
extern int _kill(pid_t pid, int signum);
extern int _sigaction(int sig, struct sigaction *new, struct sigaction *old);
