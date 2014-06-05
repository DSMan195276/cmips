/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "mips_emu/mips_emu.h"

const char *mips_reg_names_strs[32] = {
    "zero", "at", "v0", "v1", "a0",
    "a1", "a2", "a3", "t0", "t1",
    "t2", "t3", "t4", "t5", "t6",
    "t7", "s0", "s1", "s2", "s3",
    "s4", "s5", "s6", "s7", "t8",
    "t9", "k0", "k1", "gp", "sp",
    "fp", "ra"
};

void mips_dump_regs(struct mips_regs *regs)
{
    int i;
    for (i = 0; i < 32; i++) {
        printf("%s: 0x%08x\t", mips_reg_names_strs[i], regs->regs[i]);
        if ((i % 5) == 0)
            printf("\n");
    }
    printf("\n");
}

