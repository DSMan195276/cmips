/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct parser_segment {
    char *data;
    size_t alloced;
    size_t len;
    uint32_t addr;
};

struct parser {
    struct parser_segment text;
    struct parser_segment data;

    uint32_t gp_addr;
};

void parser_init(struct parser *);
void parser_clear(struct parser *);

/* Matches based on the file extension */
int (*parser_get_correct_func(const char *filename))
    (struct parser *, FILE *);

#include "parser/in/asm.h"
#include "parser/out/asm.h"

#endif
