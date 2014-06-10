/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_ASM_TOKENIZER_LEXER_H
#define INCLUDE_ASM_TOKENIZER_LEXER_H

#include <stdint.h>
#include <stdio.h>

enum asm_token {
#define X(id) id,
# include "tokenizer_lexer.x"
#undef X
};

enum asm_dir_types {
#define X(dir) DIR_##dir,
# include "tokenizer_lexer_dir.x"
#undef X
};

struct tokenizer {
    int line;
    int id_len, id_alloc;
    char *ident;
    uint32_t val;
};

enum asm_token yylex(struct tokenizer *);
extern FILE *yyin;
extern char *yytext;
extern int yylex_destroy(void);

extern const char *asm_tok_types_str[];
extern const char *asm_dir_types_str[];

#endif
