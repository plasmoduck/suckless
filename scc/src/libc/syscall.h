extern void *_brk(void *addr);
extern int _open(const char *path, int flags, int mode);
extern int _close(int fd);
extern int _read(int fd, void *buf, size_t n);
extern int _write(int fd, void *buf, size_t n);
extern int _lseek(int fd, long off, int whence);
extern void _Exit(int status);
extern int _access(char *path, int mode);
extern int _unlink(const char *path);
