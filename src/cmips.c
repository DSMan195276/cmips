/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mips_emu.h"
#include "asm.h"

int main(int argc, char **argv)
{
    struct mips_emu emu;
    struct asm_gen gen;
    int i;

    /* uint32_t inst = mips_create_i_format(OP_ADDI, REG_T1, REG_T1, 20); */

    memset(&emu, 0, sizeof(struct mips_emu));
    memset(&gen, 0, sizeof(struct asm_gen));

    asm_gen_from_file(&gen, "./test.asm");

    mips_dump_regs(&emu.r);

    for (i = 0; i < gen.text_size / 4; i++) {
        mips_disp_inst(((uint32_t *)gen.text)[i]);
        mips_run_inst(&emu, ((uint32_t *)gen.text)[i]);
    }

    mips_dump_regs(&emu.r);
    return 0;
}

