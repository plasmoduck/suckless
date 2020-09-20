POWERPC = target/powerpc
POWERPC_OBJ =\
	$(OBJS)\
	$(POWERPC)/powerpctbl.o\
	$(POWERPC)/powerpc.o\
	$(POWERPC)/ins.o\

$(POWERPC)/powerpctbl.c: $(POWERPC)/ops.dat $(POWERPC)/opers.dat
	./mktbl -f powerpc -c powerpc

$(LIBEXEC)/as-powerpc: $(POWERPC_OBJ)
	$(CC) $(SCC_LDFLAGS) $(POWERPC_OBJ) -lscc -o $@
