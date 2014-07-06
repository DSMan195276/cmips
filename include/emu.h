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

#include <stdio.h>
#include <stdarg.h>

#include "mips.h"
#include "emu/mem.h"
#include "parser.h"

/* This is an actual MIPS machine state. One of these should be created and
 * then sent to mips_emu_init() to be initalized. When you're done with it,
 * call mips_emu_clear to toss out any memory it may be holding on to. */
struct emulator {
    struct mips_regs r;
    struct mem_prog  mem;

    unsigned int stop_prog :1;

    struct parser_segment backup_text;
    struct parser_segment backup_data;

    int infd, outfd;

    void (*err_disp) (const char *err, va_list args);
};

void emulator_init(struct emulator *);
void emulator_clear(struct emulator *);

void emulator_run_inst(struct emulator *, uint32_t inst);
void emulator_run_next_inst(struct emulator *);
void emulator_run(struct emulator *);

void emulator_reset(struct emulator *);

void emulator_load_from_parser(struct emulator *, struct parser *);

int emulator_load_asm(struct emulator *emu, FILE *file);
int emulator_load_from_file(struct emulator *emu, const char *filename);

#endif
