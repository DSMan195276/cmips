/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "mips_emu.h"
#include "special.h"

static void op_func_add(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)(emu->r.regs[rs]) + (int32_t)(emu->r.regs[rt]);
}

static void op_func_addu(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) + (uint32_t)(emu->r.regs[rt]);
}

static void op_func_sub(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)(emu->r.regs[rs]) - (int32_t)(emu->r.regs[rt]);
}

static void op_func_subu(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) - (uint32_t)(emu->r.regs[rt]);
}

static void (*op_special_func_jmp_table[64])(struct mips_emu *emu, int rs, int rt, int rd, int sa) = {
    [OP_FUNC_ADD] = op_func_add,
    [OP_FUNC_ADDU] = op_func_addu,
    [OP_FUNC_SUB] = op_func_sub,
    [OP_FUNC_SUBU] = op_func_subu,
};

void op_special(struct mips_emu *emu, uint32_t inst)
{
    int func = INST_R_FUNC(inst);
    int rs = INST_R_RS(inst);
    int rt = INST_R_RT(inst);
    int rd = INST_R_RD(inst);
    int sa = INST_R_SA(inst);

    void (*f)(struct mips_emu *, int, int, int, int) = op_special_func_jmp_table[func];

    if (f)
        f(emu, rs, rt, rd, sa);
}

