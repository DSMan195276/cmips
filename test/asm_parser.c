/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>
#include <stdint.h>

#include "test.h"
#include "mips.h"
#include "parser.h"

struct test_inst {
    const uint32_t inst;
    const char *instname;
};

#define DEF_I_FORMAT(op, name) {               \
    .inst = mips_create_i_format(op, 0, 0, 0), \
    .instname = #name,                         \
}

#define DEF_R_FORMAT(func, name) {                              \
    .inst = mips_create_r_format(OP_SPECIAL, 0, 0, 0, 0, func), \
    .instname = #name,                                          \
}

#define DEF_J_FORMAT(op, name) {         \
    .inst = mips_create_j_format(op, 0), \
    .instname = #name,                   \
}

static struct test_inst asm_inst[] = {
    DEF_R_FORMAT(OP_FUNC_SLL, sll),
    DEF_R_FORMAT(OP_FUNC_SRL, srl),
    DEF_R_FORMAT(OP_FUNC_SRA, sra),

    DEF_R_FORMAT(OP_FUNC_SLLV, sllv),
    DEF_R_FORMAT(OP_FUNC_SRLV, srlv),
    DEF_R_FORMAT(OP_FUNC_SRAV, srav),

    DEF_R_FORMAT(OP_FUNC_JR, jr),
    DEF_R_FORMAT(OP_FUNC_JALR, jalr),

    DEF_R_FORMAT(OP_FUNC_ADD, add),
    DEF_R_FORMAT(OP_FUNC_ADDU, addu),
    DEF_R_FORMAT(OP_FUNC_SUB, sub),
    DEF_R_FORMAT(OP_FUNC_SUBU, subu),
    DEF_R_FORMAT(OP_FUNC_AND, and),
    DEF_R_FORMAT(OP_FUNC_OR, or),
    DEF_R_FORMAT(OP_FUNC_XOR, xor),
    DEF_R_FORMAT(OP_FUNC_NOR, nor),
    DEF_R_FORMAT(OP_FUNC_SLT, slt),
    DEF_R_FORMAT(OP_FUNC_SLTU, sltu),

    DEF_R_FORMAT(OP_FUNC_SYSCALL, syscall),
    DEF_R_FORMAT(OP_FUNC_BREAK, break),

    DEF_J_FORMAT(OP_J, j),
    DEF_J_FORMAT(OP_JAL, jal),

    DEF_I_FORMAT(OP_BEQ, beq),
    DEF_I_FORMAT(OP_BNE, bne),

    DEF_I_FORMAT(OP_ADDI, addi),
    DEF_I_FORMAT(OP_ADDIU, addiu),
    DEF_I_FORMAT(OP_SLTI, slti),
    DEF_I_FORMAT(OP_SLTIU, sltiu),
    DEF_I_FORMAT(OP_ANDI, andi),
    DEF_I_FORMAT(OP_ORI, ori),
    DEF_I_FORMAT(OP_XORI, xori),

    DEF_I_FORMAT(OP_LUI, lui),

    DEF_I_FORMAT(OP_LB, lb),
    DEF_I_FORMAT(OP_LH, lh),
    DEF_I_FORMAT(OP_LWL, lwl),
    DEF_I_FORMAT(OP_LW, lw),
    DEF_I_FORMAT(OP_LBU, lbu),
    DEF_I_FORMAT(OP_LHU, lhu),
    DEF_I_FORMAT(OP_LWR, lwr),
    DEF_I_FORMAT(OP_SB, sb),
    DEF_I_FORMAT(OP_SH, sh),
    DEF_I_FORMAT(OP_SWL, swl),
    DEF_I_FORMAT(OP_SW, sw),
    DEF_I_FORMAT(OP_SWR, swr),

    DEF_R_FORMAT(0, nop),
    DEF_R_FORMAT(0, noop),

    { 0, NULL }
};

static int assert_with_name(const char *name, const char *arg, int line, int cond)
{
    char buf[256];
    strcpy(buf, name);
    strcat(buf, ": ");
    strcat(buf, arg);
    return assert_true(buf, line, cond);
}

#define test_assert_with_name(name, cond) assert_with_name(name, #cond, __LINE__, !!(cond))

int test_comp_asm(void)
{
    struct test_inst *inst = asm_inst;
    struct parser parser;
    int ret = 0, r, i = 0, comp;
    size_t len;

    parser_init(&parser);

    r = parser_load_asm_file(&parser, "./test/asm/cmds.s");
    ret += test_assert_with_name("parser", r == 0);

    len = (sizeof(asm_inst) / (sizeof(asm_inst[0])) - 1) * 4;
    ret += test_assert_with_name("parser", parser.text.len == len);

    for (; inst->instname != NULL; inst++, i++) {

        comp = be32_to_cpu(((be32*)parser.text.data)[i]);
        ret += test_assert_with_name(inst->instname, comp == inst->inst);
    }

    parser_clear(&parser);

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { test_comp_asm, "asm parser" }
    };

    ret = run_tests("Test asm parser", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}

