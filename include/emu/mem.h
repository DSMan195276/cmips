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
    uint32_t addr;
    uint32_t size;
    char *block;
};

struct mem_prog {
    union {
        struct {
            struct mem_chunk text;
            struct mem_chunk data;
            struct mem_chunk stack;
        };
        struct mem_chunk mem[3];
    };
};

void mem_prog_init(struct mem_prog *);
void mem_prog_clear(struct mem_prog *);

/* Stack grows downward - addr should be the highest addr of the stack */
void mem_set_stack(struct mem_prog *prog, uint32_t addr, int size);

/* Note - These functions don't make copies. 'text' and 'data' should be 
 * malloc allocated and will be freed by mem_prog_clear */
void mem_set_text(struct mem_prog *prog, uint32_t addr, int size, void *text);
void mem_set_data(struct mem_prog *prog, uint32_t addr, int size, void *data);

void mem_write_to_addr(struct mem_prog *, uint32_t addr, int len, void *buf);

void mem_read_from_addr(struct mem_prog *, uint32_t addr, int len, void *buf);

#endif
