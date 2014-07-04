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
#include "syscall.h"
#include "special.h"

static void op_func_sll(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)emu->r.regs[rt] << sa;
}

static void op_func_sra(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)emu->r.regs[rt] >> sa;
}

static void op_func_srl(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)emu->r.regs[rt] >> sa;
}

static void op_func_sllv(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)emu->r.regs[rt] << (int32_t)(emu->r.regs[sa]);
}

static void op_func_srav(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)emu->r.regs[rt] >> (int32_t)(emu->r.regs[sa]);
}

static void op_func_srlv(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)emu->r.regs[rt] >> (int32_t)(emu->r.regs[sa]);
}

static void op_func_add(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)(emu->r.regs[rs]) + (int32_t)(emu->r.regs[rt]);
}

static void op_func_addu(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) + (uint32_t)(emu->r.regs[rt]);
}

static void op_func_sub(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)(emu->r.regs[rs]) - (int32_t)(emu->r.regs[rt]);
}

static void op_func_subu(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) - (uint32_t)(emu->r.regs[rt]);
}

static void op_func_and(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) & (uint32_t)(emu->r.regs[rt]);
}

static void op_func_or(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) | (uint32_t)(emu->r.regs[rt]);
}

static void op_func_xor(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) ^ (uint32_t)(emu->r.regs[rt]);
}

static void op_func_nor(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = ~((uint32_t)(emu->r.regs[rs]) | (uint32_t)(emu->r.regs[rt]));
}

static void op_func_jr(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.pc = emu->r.regs[rs];
}

static void op_func_jalr(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[REG_RA] = emu->r.pc;
    emu->r.pc = emu->r.regs[rs];
}

static void op_func_slt(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (int32_t)(emu->r.regs[rs]) < (int32_t)(emu->r.regs[rt]);
}

static void op_func_sltu(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = (uint32_t)(emu->r.regs[rs]) < (uint32_t)(emu->r.regs[rt]);
}

static void op_func_mfhi(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = emu->r.hi;
}

static void op_func_mflo(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.regs[rd] = emu->r.lo;
}

static void op_func_mthi(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.hi = emu->r.regs[rd];
}

static void op_func_mtlo(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    emu->r.lo = emu->r.regs[rd];
}

static void op_func_nop(struct emulator *emu, int rs, int rt, int rd, int sa)
{

}

static void (*op_special_func_jmp_table[64])(struct emulator *emu, int rs, int rt, int rd, int sa) = {
#define X(op, code, func) [OP_FUNC_##op] = func,
# include "mips/function.x"
#undef X
};

void op_special(struct emulator *emu, int rs, int rt, int rd, int sa, int func)
{
    void (*f)(struct emulator *, int, int, int, int) = op_special_func_jmp_table[func];

    if (f)
        f(emu, rs, rt, rd, sa);
}

