/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_MIPS_EMU_MEM_H
#define INCLUDE_MIPS_EMU_MEM_H
#include "common.h"

#include <stdint.h>

struct mem_chunk {
    uint32_t start;
    uint32_t size;
    char *block;
};

struct mem_prog {
    int chunk_count;
    struct mem_chunk *chunks;
};

void mem_prog_init(struct mem_prog *);
void mem_prog_clear(struct mem_prog *);

void mem_write_to_addr(struct mem_prog *, uint32_t addr, char *data, int len);

char *mem_read_from_addr(struct mem_prog *, uint32_t addr, int len);

#endif
