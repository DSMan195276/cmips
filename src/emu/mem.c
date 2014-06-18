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

#include "mips.h"
#include "emu/mem.h"

void mem_prog_init(struct mem_prog *prog)
{
    memset(prog, 0, sizeof(struct mem_prog));
}

void mem_prog_clear(struct mem_prog *prog)
{
    free(prog->text.block);
    free(prog->data.block);
    free(prog->stack.block);
}

void mem_set_stack(struct mem_prog *prog, uint32_t addr, int size)
{
    free(prog->stack.block);
    prog->stack.addr = addr - size;
    prog->stack.size = size;
    prog->stack.block = malloc(size);
}

void mem_set_text(struct mem_prog *prog, uint32_t addr, int size, void *text)
{
    free(prog->text.block);
    prog->text.addr = addr;
    prog->text.size = size;
    prog->text.block = text;
}

void mem_set_data(struct mem_prog *prog, uint32_t addr, int size, void *data)
{
    free(prog->data.block);
    prog->data.addr = addr;
    prog->data.size = size;
    prog->data.block = data;
}

void mem_write_to_addr(struct mem_prog *prog, uint32_t addr, int len, void *buf)
{
    int i;
    for (i = 0; i < sizeof(prog->mem) / sizeof(prog->mem[0]); i++) {
        if (prog->mem[i].addr <= addr && prog->mem[i].addr + prog->mem[i].size >= addr) {
            if (len <= prog->mem[i].size - (addr - prog->mem[i].addr))
                memcpy(prog->mem[i].block + (addr - prog->mem[i].addr), buf, len);
            else
                memcpy(prog->mem[i].block + (addr - prog->mem[i].addr), buf,
                        prog->mem[i].size - (addr - prog->mem[i].addr));
            break;
        }
    }
}

void mem_read_from_addr(struct mem_prog *prog, uint32_t addr, int len, void *buf)
{
    int i;
    for (i = 0; i < sizeof(prog->mem) / sizeof(prog->mem[0]); i++) {
        if (prog->mem[i].addr <= addr && prog->mem[i].addr + prog->mem[i].size >= addr) {
            if (len <= prog->mem[i].size - (addr - prog->mem[i].addr))
                memcpy(buf, prog->mem[i].block + (addr - prog->mem[i].addr), len);
            else
                memcpy(buf, prog->mem[i].block + (addr - prog->mem[i].addr),
                        prog->mem[i].size - (addr - prog->mem[i].addr));
            break;
        }
    }
    return ;
}

