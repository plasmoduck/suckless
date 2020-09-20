I386_OBJ =\
	$(OBJS)\
	target/x86/i386tbl.o\
	target/x86/i386.o\
	target/x86/ins.o\

target/x86/i386tbl.c: target/x86/ops.dat target/x86/opers.dat
	./mktbl -f x86 -c i386

$(LIBEXEC)/as-i386: $(I386_OBJ)
	$(CC) $(SCC_LDFLAGS) $(I386_OBJ) -lscc -o $@
