I286_OBJ =\
	$(OBJS)\
	target/x86/i286tbl.o\
	target/x86/i286.o\
	target/x86/ins.o\

target/x86/i286tbl.c: target/x86/ops.dat target/x86/opers.dat
	./mktbl -f x86 -c i286

$(LIBEXEC)/as-i286: $(I286_OBJ)
	$(CC) $(SCC_LDFLAGS) $(I286_OBJ) -lscc -o $@
