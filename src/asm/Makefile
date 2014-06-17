OBJS_y += asm.o assembler.o dir_parse.o inst_parse.o label_parse.o dir_types.o

OBJS_y += lexer.o

CLEAN_LIST_y += lexer.c

$(objtree)/lexer.c: $(srctree)/lexer.l
	@echo " LEX     $@"
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

$(objtree)/lexer.o: $(objtree)/lexer.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


