OBJ-amd64-sysv = $(OBJS)  \
        target/amd64-sysv/cgen.o \
        target/amd64-sysv/optm.o \
        target/amd64-sysv/code.o \
        target/amd64-sysv/types.o

$(LIBEXEC)/cc2-amd64-sysv: $(OBJ-amd64-sysv)
	$(CC) $(SCC_LDFLAGS) $(OBJ-amd64-sysv) -lscc -o $@
