#deps
./linux/_getheap.o: ./linux/../../linux/_getheap.c
./linux/_open.o: ./linux/../../../syscall.h
./linux/_tzone.o: ./linux/../../posix/_tzone.c
./linux/getenv.o: ./linux/../../posix/getenv.c
./linux/raise.o: ./linux/../../posix/raise.c
./linux/signal.o: ./linux/../../posix/signal.c
./linux/time.o: ./linux/../../posix/time.c
./memchr.o: ./../../string/memchr.c
./memcmp.o: ./../../string/memcmp.c
./memcpy.o: ./../../string/memcpy.c
./memmove.o: ./../../string/memmove.c
./memset.o: ./../../string/memset.c
