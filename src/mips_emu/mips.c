/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include <stdio.h>
#include <string.h>

#include "mips_emu.h"
#include "special.h"

static void op_jmp(struct mips_emu *emu, uint32_t inst)
{

}

static void op_jmp_load(struct mips_emu *emu, uint32_t inst)
{

}

static void op_addi(struct mips_emu *emu, uint32_t inst)
{
    int rs = INST_I_RS(inst);
    int rt = INST_I_RT(inst);
    int16_t val = INST_I_OFFSET(inst);

    emu->r.regs[rt] = emu->r.regs[rs] + val;
}

static void (*op_jmp_table[64])(struct mips_emu *emu, uint32_t inst) = {
    [OP_SPECIAL] = op_special,
    [OP_J] = op_jmp,
    [OP_JAL] = op_jmp_load,
    [OP_ADDI] = op_addi,
};

enum inst_type mips_opcode_to_type[64] = {
    [OP_SPECIAL] = R_FORMAT,
    [OP_J] = J_FORMAT,
    [OP_JAL] = J_FORMAT,
    [OP_BEQ] = I_FORMAT,
    [OP_BNE] = I_FORMAT,
    [OP_BLEZ] = I_FORMAT,
    [OP_BGEZ] = I_FORMAT,
    [OP_ADDI ... OP_LUI] = I_FORMAT,
    [OP_LB ... OP_LWR] = I_FORMAT,
    [OP_SB ... OP_SW] = I_FORMAT,
    [OP_SWR] = I_FORMAT
};

const char *mips_opcode_names[64] = {
#define X(op, val) [OP_##op] = #op,
# include "mips_emu_opcode.h"
#undef X
};

const char *mips_function_names[64] = {
#define X(op, val) [OP_FUNC_##op] = #op,
# include "mips_emu_function.h"
#undef X
};

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

        printf("R_FMT: rs: 0x%02x($%s), rt: 0x%02x($%s)\n-        rd: 0x%02x($%s), sa: 0x%02x, func: 0x%02x(%s)\n"
                , rs, mips_reg_names_strs[rs]
                , rt, mips_reg_names_strs[rt]
                , rd, mips_reg_names_strs[rd]
                , sa
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

void mips_run_next_inst(struct mips_emu *emu)
{
    emu->r.pc += 4;
}

void mips_run_inst(struct mips_emu *emu, uint32_t inst)
{
    int op = INST_OPCODE(inst);
    void (*f)(struct mips_emu *, uint32_t) = op_jmp_table[op];

    if (f)
        f(emu, inst);
}

void mips_emu_init(struct mips_emu *emu)
{
    memset(emu, 0, sizeof(struct mips_emu));
}

void mips_emu_clear(struct mips_emu *emu)
{

}

