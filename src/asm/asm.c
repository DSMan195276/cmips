/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips.h"
#include "assembler.h"
#include "asm.h"

void asm_init(struct asm_gen *gen)
{
    memset(gen, 0, sizeof(struct asm_gen));
}

void asm_clear(struct asm_gen *gen)
{
    free(gen->text.data);
    free(gen->data.data);
}

char *asm_escape_string(char *str)
{
    char *res;
    int cur = 0;
    int alloced = strlen(str) + 20;
    res = malloc(alloced);

    for (; *str; str++) {
        if (cur + 4 >= alloced) {
            alloced += 20;
            res = realloc(res, alloced);
        }

        switch (*str) {
        case '\t':
        case '\r':
        case '\n':
            res[cur++] = '\\';
            res[cur++] = 'x';
            sprintf(res + cur, "%02x", *str);
            cur += 2;
            break;
        default:
            res[cur++] = *str;
            break;
        }
    }

    if (cur + 1 >= alloced) {
        alloced += 1;
        res = realloc(res, alloced);
    }
    res[cur] = '\0';

    return res;
}

int asm_gen_from_file(struct asm_gen *gen, const char *filename)
{
    gen->gp_addr = gen->data.addr + 0x8000;
    return assemble_prog(gen, filename);
}

