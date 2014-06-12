OBJS_y += asm.o assembler.o dir_parse.o inst_parse.o label_parse.o tokenizer_dir_types.o

OBJS_y += tokenizer_lexer.o

CLEAN_LIST_y += tokenizer_lexer.c

$(objtree)/tokenizer_lexer.c: $(srctree)/tokenizer_lexer.l
	@echo " LEX     $@"
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

$(objtree)/tokenizer_lexer.o: $(objtree)/tokenizer_lexer.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


