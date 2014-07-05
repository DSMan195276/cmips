/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "rbtree.h"
#include "assembler_internal.h"
#include "label_parse.h"

enum internal_ret parse_label(struct assembler *a)
{
    struct label_list *label;
    struct lexer_link *link = a->link;
    size_t size = sizeof(struct label_list) + strlen(link->lex.ident) + 1;

    label = malloc(size);
    memset(label, 0, size);

    if (a->cur_section == SECT_TEXT)
        label->addr = a->text.last_addr;
    else
        label->addr = a->data.last_addr;
    strcpy(label->ident, link->lex.ident);

    if (!rb_insert(&a->labels, &label->node)) {
        parser_disp_err(a->gen, "Error: Duplicate label '%s' on line %d\n", label->ident, link->lex.line);
        free(label);
        return RET_ERR;
    }

    return RET_CONTINUE;
}

