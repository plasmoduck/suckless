typedef int sig_atomic_t;

#define SIG_ERR    ((void (*)(int))-1)
#define SIG_DFL    ((void (*)(int)) 0)
#define SIG_IGN    ((void (*)(int)) 1)

#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGABRT     6
#define SIGFPE      8
#define SIGKILL     9
#define SIGSEGV    11
#define SIGPIPE    13
#define SIGALRM    14
#define SIGTERM    15
#define SIGSTOP    17
#define SIGTSTP    18
#define SIGCONT    19
#define SIGCHLD    20
#define SIGTTIN    21
#define SIGTTOU    22
#define SIGUSR1    30
#define SIGUSR2    31

#define __NR_SIGNALS 32
