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
# include "lexer.x"
#undef X
};

enum asm_dir {
    DIR_NONE = -1,
#define X(str, dir, func, sect) DIR_##dir,
# include "lexer_dir.x"
#undef X
};

struct lexer {
    int line;
    int id_len, id_alloc;
    char *ident;
    uint32_t val;
};

enum asm_token yylex(struct lexer *);
extern FILE *yyin;
extern char *yytext;
extern int yylex_destroy(void);

extern const char *asm_tok_types_str[];
extern const char *asm_dir_types_str[];

#endif
