/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_ASM_TOKENIZER_H
#define INCLUDE_ASM_TOKENIZER_H

#include <stdio.h>

#include "lex/tokenizer_lexer.h"

struct token_list {
    struct token_list *next;
    enum asm_token tok;
    char *ident;
    int line;
    int val;
};

struct token_list *tokenizer_run(FILE *file);

void tokenizer_free_tokens(struct token_list *);

#endif
