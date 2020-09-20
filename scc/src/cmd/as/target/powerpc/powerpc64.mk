POWERPC = target/powerpc
POWERPC64_OBJ =\
	$(OBJS)\
	$(POWERPC)/powerpc64tbl.o\
	$(POWERPC)/powerpc64.o\
	$(POWERPC)/ins.o\

$(POWERPC)/powerpc64tbl.c: $(POWERPC)/ops.dat $(POWERPC)/opers.dat
	./mktbl -f powerpc -c powerpc64

$(LIBEXEC)/as-powerpc64: $(POWERPC64_OBJ)
	$(CC) $(SCC_LDFLAGS) $(POWERPC64_OBJ) -lscc -o $@
