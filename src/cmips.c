/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>

#include "mips.h"
#include "emu.h"
#include "asm.h"
#include "args.h"
#include "input.h"

struct mips_emu cmips_emu;
struct asm_gen  cmips_asm_gen;
struct arg_state cmips_arg_state;

int main(int argc, char **argv)
{

    mips_emu_init(&cmips_emu);
    asm_init(&cmips_asm_gen);

    cmips_asm_gen.lowest_addr = 0x00100000;

    parse_args(argc, argv, &cmips_arg_state);

    if (!cmips_arg_state.quiet)
        printf("%s", version_text);

    if (!cmips_arg_state.noinput)
        run_input_loop();

    asm_clear(&cmips_asm_gen);
    mips_emu_clear(&cmips_emu);

    return 0;
}

