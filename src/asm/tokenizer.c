/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asm/tokenizer.h"
#include "asm/tokenizer_lexer.h"

struct token_list *tokenizer_run(FILE *file)
{
    enum asm_token tok;
    struct tokenizer tokenizer;
    struct token_list *head = NULL, *last, *l;
    yyin = file;

    memset(&tokenizer, 0, sizeof(struct tokenizer));

    tokenizer.line = 1;

    do {
        tok = yylex(&tokenizer);

        l = malloc(sizeof(struct token_list));
        memset(l, 0, sizeof(struct token_list));
        l->tok = tok;
        l->ident = tokenizer.ident;
        l->line = tokenizer.line;
        l->val = tokenizer.val;
        tokenizer.ident = NULL;
        if (!head) {
            head = l;
            last = l;
        } else {
            last->next = l;
            last = l;
        }
    } while (tok != TOK_EOF);

    return head;
}

void tokenizer_free_tokens(struct token_list *t)
{
    struct token_list *tmp, *l;
    for (l = t; l != NULL; l = tmp) {
        tmp = l->next;
        free(l->ident);
        free(l);
    }
}

