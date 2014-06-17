/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H
#include "common.h"

#include <stdint.h>

struct segment {
    char *data;
    size_t alloced;
    size_t len;
    uint32_t addr;
};

struct asm_gen {
    struct segment text;
    struct segment data;

    uint32_t gp_addr;
};

void asm_init(struct asm_gen *);
void asm_clear(struct asm_gen *);

int asm_gen_from_file(struct asm_gen *, const char *filename);

/* Must free result */
char *asm_escape_string(char *str);

#endif
