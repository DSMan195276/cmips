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

    label = malloc(sizeof(struct label_list) + strlen(a->tokenizer.ident) + 1);

    if (a->cur_section == SECT_TEXT)
        label->addr = a->text.last_addr;
    else
        label->addr = a->data.last_addr;
    strcpy(label->ident, a->tokenizer.ident);

    printf("Adding label: %s\n", label->ident);
    rb_insert(&a->labels, &label->node);

    return RET_CONTINUE;
}

