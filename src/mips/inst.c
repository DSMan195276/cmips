/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include "mips.h"
#include "mips/inst.h"

#define ID_3_OPER_SPEC(str, funcn) {                       \
    {                                                      \
        .ident = str,                                      \
        .reg_count = 3,                                    \
        .rs = { REG_REGISTER, REG_REGISTER, REG_REGISTER } \
    },                                                     \
    .format = R_FORMAT,                                    \
    .opcode = OP_SPECIAL,                                  \
    .func = (funcn),                                       \
    .place = { REGP_RD, REGP_RS, REGP_RT }                 \
}

#define ID_3_OPER_SPEC_NONE(str, funcn) { \
    {                                     \
        .ident = str,                     \
        .reg_count = 0,                   \
        .rs = { 0 }                       \
    },                                    \
    .format = R_FORMAT,                   \
    .opcode = OP_SPECIAL,                 \
    .func = (funcn),                      \
    .place = { 0 }                        \
}

#define ID_3_OPER_JR(str, funcn) { \
    {                              \
        .ident = str,              \
        .reg_count = 1,            \
        .rs = { REG_REGISTER },    \
    },                             \
    .format = R_FORMAT,            \
    .opcode = OP_SPECIAL,          \
    .func = (funcn),               \
    .place = { REGP_RS }           \
}

#define ID_3_OPER_I(str, op) {                               \
    {                                                        \
        .ident = str,                                        \
        .reg_count = 3,                                      \
        .rs = { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, \
    },                                                       \
    .format = I_FORMAT,                                      \
    .opcode = op,                                            \
    .func = 0,                                               \
    .place = { REGP_RT, REGP_RS, REGP_IMMEDIATE },           \
    .addr_is_branch = 1,                                     \
    .addr_bits = 16,                                         \
    .addr_shift = 0,                                         \
    .addr_mask = 0x0000FFFF,                                 \
}

#define ID_SHIFT(str, funcn) {                               \
    {                                                        \
        .ident = str,                                        \
        .reg_count = 3,                                      \
        .rs = { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, \
    },                                                       \
    .format = R_FORMAT,                                      \
    .opcode = OP_SPECIAL,                                    \
    .func = funcn,                                           \
    .place = { REGP_RD, REGP_RT, REGP_SA }                   \
}

#define ID_SHIFT_VAR(str, funcn) {                          \
    {                                                       \
        .ident = str,                                       \
        .reg_count = 3,                                     \
        .rs = { REG_REGISTER, REG_REGISTER, REG_REGISTER }, \
    },                                                      \
    .format = R_FORMAT,                                     \
    .opcode = OP_SPECIAL,                                   \
    .func = funcn,                                          \
    .place = { REGP_RD, REGP_RT, REGP_SA }                  \
}

#define ID_LUI(str, op) {                      \
    {                                          \
        .ident = str,                          \
        .reg_count = 2,                        \
        .rs = { REG_REGISTER, REG_IMMEDIATE }, \
    },                                         \
    .format = I_FORMAT,                        \
    .opcode = op,                              \
    .func = 0,                                 \
    .place = { REGP_RT, REGP_IMMEDIATE }       \
}

#define ID_BRANCH(str, op) {                              \
    {                                                     \
        .ident = str,                                     \
        .reg_count = 3,                                   \
        .rs = { REG_REGISTER, REG_REGISTER, REG_ADDRESS}, \
    },                                                    \
    .format = I_FORMAT,                                   \
    .opcode = op,                                         \
    .func = 0,                                            \
    .place = { REGP_RS, REGP_RT, REGP_IMMEDIATE },        \
    .addr_is_branch = 2,                                  \
    .addr_bits = 16,                                      \
    .addr_shift = 2,                                      \
    .addr_mask = 0xFFFFFFFC                               \
}

#define ID_J(str, op) {        \
    {                          \
        .ident = str,          \
        .reg_count = 1,        \
        .rs = { REG_ADDRESS }, \
    },                         \
    .format = J_FORMAT,        \
    .opcode = op,              \
    .func = 0,                 \
    .place = { REGP_ADDRESS }, \
    .addr_is_branch = 1,       \
    .addr_bits = 26,           \
    .addr_shift = 2,           \
    .addr_mask = 0xFFFFFFFC    \
}

#define ID_MEM(str, op) {                                     \
    {                                                         \
        .ident = str,                                         \
        .reg_count = 3,                                       \
        .rs = { REG_REGISTER, REG_DEREF_REG, REG_DEREF_REG }, \
    },                                                        \
    .format = I_FORMAT,                                       \
    .opcode = op,                                             \
    .func = 0,                                                \
    .place = { REGP_RT, REGP_IMMEDIATE, REGP_RS }             \
}

#define ID_MV(str, funcop) {                  \
    {                                         \
        .ident = str,                         \
        .reg_count = 1,                       \
        .rs = { REG_REGISTER },               \
    },                                        \
    .format = R_FORMAT,                       \
    .opcode = OP_SPECIAL,                     \
    .func = funcop,                           \
    .place = { REGP_RD }                      \
}

#define ID_MUL_DIV(str, funcop) {             \
    {                                         \
        .ident = str,                         \
        .reg_count = 2,                       \
        .rs = { REG_REGISTER, REG_REGISTER }, \
    },                                        \
    .format = R_FORMAT,                       \
    .opcode = OP_SPECIAL,                     \
    .func = funcop,                           \
    .place = { REGP_RS, REGP_RT }             \
}


const struct inst_desc inst_ids[] = {
    ID_SHIFT("sll", OP_FUNC_SLL),
    ID_SHIFT("srl", OP_FUNC_SRL),
    ID_SHIFT("sra", OP_FUNC_SRA),

    ID_SHIFT_VAR("sllv", OP_FUNC_SLLV),
    ID_SHIFT_VAR("srlv", OP_FUNC_SRLV),
    ID_SHIFT_VAR("srav", OP_FUNC_SRAV),

    ID_3_OPER_JR("jr",   OP_FUNC_JR),
    ID_3_OPER_JR("jalr", OP_FUNC_JALR),

    ID_3_OPER_SPEC("add",  OP_FUNC_ADD),
    ID_3_OPER_SPEC("addu", OP_FUNC_ADDU),
    ID_3_OPER_SPEC("sub",  OP_FUNC_SUB),
    ID_3_OPER_SPEC("subu", OP_FUNC_SUBU),
    ID_3_OPER_SPEC("and",  OP_FUNC_AND),
    ID_3_OPER_SPEC("or",   OP_FUNC_OR),
    ID_3_OPER_SPEC("xor",  OP_FUNC_XOR),
    ID_3_OPER_SPEC("nor",  OP_FUNC_NOR),
    ID_3_OPER_SPEC("slt",  OP_FUNC_SLT),
    ID_3_OPER_SPEC("sltu", OP_FUNC_SLTU),

    ID_3_OPER_SPEC_NONE("syscall", OP_FUNC_SYSCALL),
    ID_3_OPER_SPEC_NONE("break",   OP_FUNC_BREAK),

    ID_J("j",   OP_J),
    ID_J("jal", OP_JAL),

    ID_BRANCH("beq", OP_BEQ),
    ID_BRANCH("bne", OP_BNE),

    ID_3_OPER_I("addi",  OP_ADDI),
    ID_3_OPER_I("addiu", OP_ADDIU),
    ID_3_OPER_I("slti",  OP_SLTI),
    ID_3_OPER_I("sltiu", OP_SLTIU),
    ID_3_OPER_I("andi",  OP_ANDI),
    ID_3_OPER_I("ori",   OP_ORI),
    ID_3_OPER_I("xori",  OP_XORI),

    ID_LUI("lui", OP_LUI),

    ID_MEM("lb",  OP_LB),
    ID_MEM("lh",  OP_LH),
    ID_MEM("lw",  OP_LW),
    ID_MEM("lbu", OP_LBU),
    ID_MEM("lhu", OP_LHU),
    ID_MEM("sb",  OP_SB),
    ID_MEM("sh",  OP_SH),
    ID_MEM("sw",  OP_SW),

    ID_MV("mfhi", OP_FUNC_MFHI),
    ID_MV("mflo", OP_FUNC_MFLO),
    ID_MV("mthi", OP_FUNC_MTHI),
    ID_MV("mtlo", OP_FUNC_MTLO),

    ID_MUL_DIV("mult", OP_FUNC_MULT),
    ID_MUL_DIV("multu", OP_FUNC_MULTU),
    ID_MUL_DIV("div", OP_FUNC_DIV),
    ID_MUL_DIV("divu", OP_FUNC_DIVU),

    { { "nop", 0, { 0 } }, R_FORMAT, 0, 0, { 0 } },
    { { "noop", 0, { 0 } }, R_FORMAT, 0, 0, { 0 } },
    { { NULL } }
};

enum inst_type mips_opcode_to_type[64] = {
#define X(op, code, fmt, func) [OP_##op] = fmt,
# include "mips/opcode.x"
#undef X
};

const char *mips_opcode_names[64] = {
#define X(op, val, fmt, func) [OP_##op] = #op,
# include "mips/opcode.x"
#undef X
};

const char *mips_function_names[64] = {
#define X(op, val, func) [OP_FUNC_##op] = #op,
# include "mips/function.x"
#undef X
};

uint32_t inst_gen(const struct inst_desc *id, struct inst_reg *regs)
{
    struct inst_reg empty = { 0 };
    struct inst_reg *rt = &empty;
    struct inst_reg *rd = &empty;
    struct inst_reg *rs = &empty;
    struct inst_reg *sa = &empty;
    struct inst_reg *imm = &empty;
    struct inst_reg *addr = &empty;
    int i = 0;

    for (; i < id->g.reg_count; i++)
        if (id->place[i] == REGP_RT)
            rt = regs + i;
        else if (id->place[i] == REGP_RS)
            rs = regs + i;
        else if (id->place[i] == REGP_RD)
            rd = regs + i;
        else if (id->place[i] == REGP_SA)
            sa = regs + i;
        else if (id->place[i] == REGP_IMMEDIATE)
            imm = regs + i;
        else if (id->place[i] == REGP_ADDRESS)
            addr = regs + i;

    if (id->format == I_FORMAT)
        return mips_create_i_format(id->opcode, rs->val, rt->val, imm->val);
    else if (id->format == R_FORMAT)
        return mips_create_r_format(id->opcode, rs->val, rt->val, rd->val, sa->val, id->func);
    else if (id->format == J_FORMAT)
        return mips_create_j_format(id->opcode, addr->val);

    return 0;
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

