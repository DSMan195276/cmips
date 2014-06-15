/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include "mips.h"
#include "emu.h"
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

static void op_func_syscall(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{
    switch (emu->r.regs[REG_V0]) {
    case 1:
        printf("%d", emu->r.regs[REG_A0]);
        break;
    case 4:
        /* printf("%s", (char *)emu->r.regs[REG_A0]); */
        break;
    default:
        break;
    }
}

static void op_func_nop(struct mips_emu *emu, int rs, int rt, int rd, int sa)
{

}

static void (*op_special_func_jmp_table[64])(struct mips_emu *emu, int rs, int rt, int rd, int sa) = {
#define X(op, code, func) [OP_FUNC_##op] = func,
# include "mips/mips_emu_function.x"
#undef X
};

void op_special(struct mips_emu *emu, int rs, int rt, int rd, int sa, int func)
{
    void (*f)(struct mips_emu *, int, int, int, int) = op_special_func_jmp_table[func];

    if (f)
        f(emu, rs, rt, rd, sa);
}

