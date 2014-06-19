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

#include "mips.h"

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
        printf("%s: 0x%08x  ", mips_reg_names_strs[i], regs->regs[i]);
        if ((i % 5) == 1)
            printf("\n");
    }
    printf("hi: 0x%08x  lo: 0x%08x  pc: 0x%08x\n",
            regs->hi, regs->lo, regs->pc);
}

void mips_disp_inst(uint32_t inst)
{
    enum inst_type t = mips_opcode_to_type[INST_OPCODE(inst)];
    printf("Inst: 0x%08x(%s)\n- ", inst, mips_opcode_names[INST_OPCODE(inst)]);
    if (t == R_FORMAT) {
        int rs = INST_R_RS(inst);
        int rt = INST_R_RT(inst);
        int rd = INST_R_RD(inst);
        int sa = INST_R_SA(inst);
        int func = INST_R_FUNC(inst);

        printf("R_FMT: rs: 0x%02x($%s), rt: 0x%02x($%s)\n         rd: 0x%02x($%s), sa: 0x%02x($%s), func: 0x%02x(%s)\n"
                , rs, mips_reg_names_strs[rs]
                , rt, mips_reg_names_strs[rt]
                , rd, mips_reg_names_strs[rd]
                , sa, mips_reg_names_strs[sa]
                , func, mips_function_names[func]);
    } else if (t == I_FORMAT) {
        int rs = INST_I_RS(inst);
        int rt = INST_I_RT(inst);
        int off = INST_I_OFFSET(inst);

        printf("I_FMT: rs: 0x%02x($%s), rt: 0x%02x($%s), off: 0x%04x(%d)\n"
                , rs, mips_reg_names_strs[rs]
                , rt, mips_reg_names_strs[rt]
                , off, off);
    } else if (t == J_FORMAT) {
        int addr = INST_J_INDEX(inst);

        printf("J_FMT: Jmp Addr: 0x%08x(%d) - Aligned: 0x%08x(%d)\n"
                , addr, addr
                , addr << 2, addr << 2);
    }
}


