INCLUDE  =\
	-I$(INCDIR)\
	-I$(INCDIR)/bits/$(SYS)\
	-I$(INCDIR)/bits/$(ARCH)\

SYSERRNO = $(INCDIR)/bits/$(SYS)/sys/errno.h

LIBC = $(LIBCDIR)/libc.a
CRT = $(LIBCDIR)/crt.o
LIBCLST= $(PROJECTDIR)/src/libc/libc.lst

MKLST = \
	echo $?  |\
	sed 's/ /\n/g' |\
	sed '/^$$/d' |\
	sed 's@^@$(PWD)/@' >> $(LIBCLST)

# Rules

.SUFFIXES: .6 .7 .8 .z .q

_sys_errlist.c: $(SYSERRNO)
	../../mkerrstr $(SYSERRNO)

$(CRT): crt.$O
	cp crt.$O $@

clean: clean-libc

clean-libc: FORCE
	rm -f *.6 *.7 *.8 *.z *.q

# amd64-posix objects
.c.6:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.6:
	$(AS) $(SCC_ASFLAGS) $< -o $@

# amd64-darwin objects
.c.6d:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.6d:
	$(AS) $(SCC_ASFLAGS) $< -o $@

# arm64-posix objects
.c.7:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.7:
	$(AS) $(SCC_ASFLAGS) $< -o $@

# 386-posix objects
.c.8:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.8:
	$(AS) $(SCC_ASFLAGS) $< -o $@

# z80 objects
.c.z:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.z:
	$(AS) $(SCC_ASFLAGS) $< -o $@

# ppc32 objects
.c.q:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

.s.q:
	$(AS) $(SCC_ASFLAGS) $< -o $@
