/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>

#include "mips_emu.h"
#include "asm.h"
#include "args.h"
#include "input.h"

struct mips_emu cmips_emu;
struct asm_gen  cmips_asm_gen;

int main(int argc, char **argv)
{
    struct arg_state arg_state = { 0 };

    mips_emu_init(&cmips_emu);
    asm_init(&cmips_asm_gen);

    cmips_asm_gen.lowest_addr = 0x00100000;

    parse_args(argc, argv, &arg_state);

    if (!arg_state.quiet)
        printf("%s", version_text);

    if (!arg_state.noinput && !arg_state.run)
        run_input_loop();

    asm_clear(&cmips_asm_gen);
    mips_emu_clear(&cmips_emu);

    return 0;
}

