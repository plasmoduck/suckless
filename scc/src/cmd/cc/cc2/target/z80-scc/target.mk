OBJ-z80-scc = $(OBJS)  \
        target/z80-scc/cgen.o \
        target/z80-scc/optm.o \
        target/z80-scc/code.o \
        target/z80-scc/types.o \

$(LIBEXEC)/cc2-z80-scc: $(OBJ-z80-scc)
	$(CC) $(SCC_LDFLAGS) $(OBJ-z80-scc) -lscc -o $@
