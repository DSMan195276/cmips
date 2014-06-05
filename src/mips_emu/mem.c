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

#include "mips_emu/mips_emu.h"
#include "mips_emu/mem.h"

void mem_prog_init(struct mem_prog *prog)
{
    memset(prog, 0, sizeof(struct mem_prog));
}

void mem_prog_clear(struct mem_prog *prog)
{
    int i;
    for (i = 0; i < prog->chunk_count; i++)
        free(prog->chunks[i].block);

    free(prog->chunks);
}

void mem_write_to_addr(struct mem_prog *prog, uint32_t addr, char *data, int len)
{

}

char *mem_read_from_addr(struct mem_prog *prog, uint32_t addr, int len)
{

    return NULL;
}

