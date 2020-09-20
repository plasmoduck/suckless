.POSIX:

ROOT   = ../../..
CFLAGS =
CC     = SCCPREFIX=$(ROOT) ./cc.sh

.c:
	$(CC) $(CFLAGS) -o $@ $<

all: tests-i386 tests-amd64

tests-i386:
	@SCCPREFIX=$(ROOT) CFLAGS='-a i386' ./chktest.sh libc-tests.lst

tests-amd64:
	@SCCPREFIX=$(ROOT) CFLAGS='-a amd64' ./chktest.sh libc-tests.lst

clean:
	rm -f *.o *core test.log `awk '{print $$1}' libc-tests.lst`
