/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>

#include "mips.h"
#include "lexer.h"
#include "asm.h"
#include "assembler_internal.h"

#define ID_3_OPER_SPEC(str, funcn) {                    \
    .ident = str,                                       \
    .format = R_FORMAT,                                 \
    .opcode = OP_SPECIAL,                               \
    .func = (funcn),                                    \
    .reg_count = 3,                                     \
    .rs = { REG_REGISTER, REG_REGISTER, REG_REGISTER }, \
    .place = { REGP_RD, REGP_RS, REGP_RT }              \
}

#define ID_3_OPER_SPEC_NONE(str, funcn) { \
    .ident = str,                         \
    .format = R_FORMAT,                   \
    .opcode = OP_SPECIAL,                 \
    .func = (funcn),                      \
    .reg_count = 0,                       \
    .rs = { 0 },                          \
    .place = { 0 }                        \
}

#define ID_3_OPER_JR(str, funcn) { \
    .ident = str,                  \
    .format = R_FORMAT,            \
    .opcode = OP_SPECIAL,          \
    .func = (funcn),               \
    .reg_count = 1,                \
    .rs = { REG_REGISTER },        \
    .place = { REGP_RS }           \
}

#define ID_3_OPER_I(str, op) {                           \
    .ident = str,                                        \
    .format = I_FORMAT,                                  \
    .opcode = op,                                        \
    .func = 0,                                           \
    .reg_count = 3,                                      \
    .rs = { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, \
    .place = { REGP_RT, REGP_RS, REGP_IMMEDIATE },       \
    .addr_is_branch = 1,                                 \
    .addr_bits = 16,                                     \
    .addr_shift = 0,                                     \
    .addr_mask = 0x0000FFFF,                             \
}

#define ID_SHIFT(str, funcn) {                           \
    .ident = str,                                        \
    .format = R_FORMAT,                                  \
    .opcode = OP_SPECIAL,                                \
    .func = funcn,                                       \
    .reg_count = 3,                                      \
    .rs = { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, \
    .place = { REGP_RD, REGP_RT, REGP_SA }               \
}

#define ID_SHIFT_VAR(str, funcn) {                      \
    .ident = str,                                       \
    .format = R_FORMAT,                                 \
    .opcode = OP_SPECIAL,                               \
    .func = funcn,                                      \
    .reg_count = 3,                                     \
    .rs = { REG_REGISTER, REG_REGISTER, REG_REGISTER }, \
    .place = { REGP_RD, REGP_RT, REGP_SA }              \
}

#define ID_LUI(str, op) {                  \
    .ident = str,                          \
    .format = I_FORMAT,                    \
    .opcode = op,                          \
    .func = 0,                             \
    .reg_count = 2,                        \
    .rs = { REG_REGISTER, REG_IMMEDIATE }, \
    .place = { REGP_RT, REGP_IMMEDIATE }   \
}

#define ID_BRANCH(str, op) {                          \
    .ident = str,                                     \
    .format = I_FORMAT,                               \
    .opcode = op,                                     \
    .func = 0,                                        \
    .reg_count = 3,                                   \
    .rs = { REG_REGISTER, REG_REGISTER, REG_ADDRESS}, \
    .place = { REGP_RS, REGP_RT, REGP_IMMEDIATE },    \
    .addr_is_branch = 2,                              \
    .addr_bits = 16,                                  \
    .addr_shift = 2,                                  \
    .addr_mask = 0xFFFFFFFC                           \
}

#define ID_J(str, op) {        \
    .ident = str,              \
    .format = J_FORMAT,        \
    .opcode = op,              \
    .func = 0,                 \
    .reg_count = 1,            \
    .rs = { REG_ADDRESS },     \
    .place = { REGP_ADDRESS }, \
    .addr_is_branch = 1,       \
    .addr_bits = 26,           \
    .addr_shift = 2,           \
    .addr_mask = 0xFFFFFFFC    \
}

#define ID_MEM(str, op) {                                 \
    .ident = str,                                         \
    .format = I_FORMAT,                                   \
    .opcode = op,                                         \
    .func = 0,                                            \
    .reg_count = 3,                                       \
    .rs = { REG_REGISTER, REG_DEREF_REG, REG_DEREF_REG }, \
    .place = { REGP_RT, REGP_IMMEDIATE, REGP_RS }         \
}

static struct inst_desc ids[] = {
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
    ID_MEM("lwl", OP_LWL),
    ID_MEM("lw",  OP_LW),
    ID_MEM("lbu", OP_LBU),
    ID_MEM("lhu", OP_LHU),
    ID_MEM("lwr", OP_LWR),
    ID_MEM("sb",  OP_SB),
    ID_MEM("sh",  OP_SH),
    ID_MEM("swl", OP_SWL),
    ID_MEM("sw",  OP_SW),
    ID_MEM("swr", OP_SWR),

    { "nop",  R_FORMAT, 0, 0, 0, { 0 }, { 0 } },
    { "noop", R_FORMAT, 0, 0, 0, { 0 }, { 0 } },
    { NULL }
};

static uint32_t gen_op(struct inst_desc *id, struct reg *regs)
{
    struct reg empty = { 0 };
    struct reg *rt = &empty;
    struct reg *rd = &empty;
    struct reg *rs = &empty;
    struct reg *sa = &empty;
    struct reg *imm = &empty;
    struct reg *addr = &empty;
    int i = 0;

    for (; i < id->reg_count; i++)
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

#define expect_token(tok, val) \
    do { \
        if ((tok) != (val)) \
            return RET_UNEXPECTED; \
    } while (0)

static void create_marker(struct assembler *a, struct inst_desc *inst)
{
    struct label_marker *m;
    size_t len = strlen(a->lexer.ident);

    m = malloc(sizeof(struct label_marker) + len + 1);
    memset(m, 0, sizeof(struct label_marker) + len + 1);
    strcpy(m->label, a->lexer.ident);

    if (a->cur_section == SECT_TEXT)
        m->addr = a->text.last_addr;
    else
        m->addr = a->data.last_addr;

    if (inst->addr_is_branch == 2)
        m->is_branch = 1;

    m->bits = inst->addr_bits;
    m->shift = inst->addr_shift;
    m->mask = inst->addr_mask;

    rb_insert(&a->markers, &m->node);
}

static enum internal_ret parse_instruction(struct assembler *a, struct inst_desc *inst)
{
    int i;
    uint32_t op;
    struct reg r[4];

    memset(r, 0, sizeof(struct reg));

    for (i = 0; i < inst->reg_count; i++) {
        a->tok = yylex(&a->lexer);

        switch (inst->rs[i]) {
        case REG_REGISTER:
            expect_token(a->tok, TOK_REGISTER);

            r[i].val = a->lexer.val;
            break;
        case REG_IMMEDIATE:
            if (a->tok == TOK_INTEGER) {
                r[i].val = a->lexer.val;
            } else if (a->tok == TOK_IDENT) {
                r[i].val = 0;
                create_marker(a, inst);
            } else {
                return RET_UNEXPECTED;
            }
            break;
        case REG_ADDRESS:
            if (a->tok == TOK_INTEGER) {
                r[i].val = a->lexer.val >> 2;
            } else if (a->tok == TOK_IDENT) {
                r[i].val = 0;
                create_marker(a, inst);
            } else {
                return RET_UNEXPECTED;
            }
            break;
        case REG_DEREF_REG:
            expect_token(a->tok, TOK_INTEGER);

            r[i].val = a->lexer.val;

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_LPAREN);

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_REGISTER);

            r[i + 1].val = a->lexer.val;
            i++;

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_RPAREN);

            break;
        }

        if (i != inst->reg_count - 1) {
            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_COMMA);
        }
    }

    op = gen_op(inst, r);

    add_word_to_seg(&a->text, op);

    return RET_CONTINUE;
}

enum internal_ret parse_command(struct assembler *a)
{
    struct inst_desc *i;

    for (i = ids; i->ident != NULL; i++)
        if (stringcasecmp(a->lexer.ident, i->ident) == 0)
            return parse_instruction(a, i);

    return RET_UNEXPECTED;
}

