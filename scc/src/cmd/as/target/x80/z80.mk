Z80_OBJ =\
	$(OBJS)\
	target/x80/z80tbl.o\
	target/x80/z80.o\
	target/x80/ins.o\

target/x80/z80tbl.c: target/x80/ops.dat target/x80/opers.dat
	./mktbl -f x80 -c z80

$(LIBEXEC)/as-z80: $(OBJ) $(Z80_OBJ)
	$(CC) $(SCC_LDFLAGS) $(Z80_OBJ) -lscc -o $@
