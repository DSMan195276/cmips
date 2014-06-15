/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_EMU
#define INCLUDE_EMU
#include "common.h"

#include "mips.h"
#include "emu/mem.h"

/* This is an actual MIPS machine state. One of these should be created and
 * then sent to mips_emu_init() to be initalized. When you're done with it,
 * call mips_emu_clear to toss out any memory it may be holding on to. */
struct mips_emu {
    struct mips_regs r;
    struct mem_prog  mem;
};

void mips_emu_init(struct mips_emu *);
void mips_emu_clear(struct mips_emu *);

void mips_run_inst(struct mips_emu *, uint32_t inst);
void mips_run_next_inst(struct mips_emu *);

#endif
