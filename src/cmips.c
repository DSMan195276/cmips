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

    cmips_asm_gen.text.addr = 0x00100000;
    /*cmips_asm_gen.data.addr = 0x01000000; */

    parse_args(argc, argv, &cmips_arg_state);

    if (cmips_arg_state.run) {
        mem_set_text(&cmips_emu.mem, cmips_asm_gen.text.addr,
                cmips_asm_gen.text.len, cmips_asm_gen.text.data);
        mem_set_data(&cmips_emu.mem, cmips_asm_gen.data.addr,
                cmips_asm_gen.data.len, cmips_asm_gen.data.data);
        cmips_asm_gen.text.data = NULL;
        cmips_asm_gen.data.data = NULL;

        cmips_emu.r.pc = cmips_asm_gen.text.addr;
        mips_run(&cmips_emu);
    }

    if (!cmips_arg_state.quiet)
        printf("%s", version_text);

    if (!cmips_arg_state.noinput)
        run_input_loop();

    asm_clear(&cmips_asm_gen);
    mips_emu_clear(&cmips_emu);

    return 0;
}

