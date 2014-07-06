objs-y += asm.o assembler.o dir_parse.o inst_parse.o label_parse.o dir_types.o pseudoinst.o

objs-y += lexer.o

clean-list-y += lexer.c

$(objtree)/lexer.c: $(srctree)/lexer.l
	@echo " LEX     $@"
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

$(objtree)/lexer.o: $(objtree)/lexer.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


