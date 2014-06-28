/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "test.h"
#include "mips.h"
#include "parser.h"

struct test_inst {
    const uint32_t inst;
    const char *instname;
    char *text;
};

#define DEF_I_FORMAT(op, name, cmd) {          \
    .inst = mips_create_i_format(op, 0, 0, 0), \
    .instname = #name,                         \
    .text = cmd                                \
}

#define DEF_R_FORMAT(func, name, cmd) {                         \
    .inst = mips_create_r_format(OP_SPECIAL, 0, 0, 0, 0, func), \
    .instname = #name,                                          \
    .text = cmd                                                 \
}

#define DEF_J_FORMAT(op, name, cmd) {    \
    .inst = mips_create_j_format(op, 0), \
    .instname = #name,                   \
    .text = cmd                          \
}

static struct test_inst asm_inst[] = {
    DEF_R_FORMAT(OP_FUNC_SLL, sll, "sll $0, $0, 0"),
    DEF_R_FORMAT(OP_FUNC_SRL, srl, "srl $0, $0, 0"),
    DEF_R_FORMAT(OP_FUNC_SRA, sra, "sra $0, $0, 0"),

    DEF_R_FORMAT(OP_FUNC_SLLV, sllv, "sllv $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SRLV, srlv, "srlv $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SRAV, srav, "srav $0, $0, $0"),

    DEF_R_FORMAT(OP_FUNC_JR, jr, "jr $0"),
    DEF_R_FORMAT(OP_FUNC_JALR, jalr, "jalr $0"),

    DEF_R_FORMAT(OP_FUNC_ADD, add, "add $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_ADDU, addu, "addu $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SUB, sub, "sub $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SUBU, subu, "subu $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_AND, and, "and $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_OR, or, "or $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_XOR, xor, "xor $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_NOR, nor, "nor $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SLT, slt, "slt $0, $0, $0"),
    DEF_R_FORMAT(OP_FUNC_SLTU, sltu,"sltu $0, $0, $0"),

    DEF_R_FORMAT(OP_FUNC_SYSCALL, syscall, "syscall"),
    DEF_R_FORMAT(OP_FUNC_BREAK, break, "break"),

    DEF_J_FORMAT(OP_J, j, "j 0"),
    DEF_J_FORMAT(OP_JAL, jal, "jal 0"),

    DEF_I_FORMAT(OP_BEQ, beq, "beq $0, $0, 0"),
    DEF_I_FORMAT(OP_BNE, bne, "bne $0, $0, 0"),

    DEF_I_FORMAT(OP_ADDI, addi, "addi $0, $0, 0"),
    DEF_I_FORMAT(OP_ADDIU, addiu, "addiu $0, $0, 0"),
    DEF_I_FORMAT(OP_SLTI, slti, "slti $0, $0, 0"),
    DEF_I_FORMAT(OP_SLTIU, sltiu, "sltiu $0, $0, 0"),
    DEF_I_FORMAT(OP_ANDI, andi, "andi $0, $0, 0"),
    DEF_I_FORMAT(OP_ORI, ori, "ori $0, $0, 0"),
    DEF_I_FORMAT(OP_XORI, xori, "xori $0, $0, 0"),

    DEF_I_FORMAT(OP_LUI, lui, "lui $0, 0"),

    DEF_I_FORMAT(OP_LB, lb, "lb $0, 0($0)"),
    DEF_I_FORMAT(OP_LH, lh, "lh $0, 0($0)"),
    DEF_I_FORMAT(OP_LWL, lwl, "lwl $0, 0($0)"),
    DEF_I_FORMAT(OP_LW, lw, "lw $0, 0($0)"),
    DEF_I_FORMAT(OP_LBU, lbu, "lbu $0, 0($0)"),
    DEF_I_FORMAT(OP_LHU, lhu, "lhu $0, 0($0)"),
    DEF_I_FORMAT(OP_LWR, lwr, "lwr $0, 0($0)"),
    DEF_I_FORMAT(OP_SB, sb, "sb $0, 0($0)"),
    DEF_I_FORMAT(OP_SH, sh, "sh $0, 0($0)"),
    DEF_I_FORMAT(OP_SWL, swl, "swl $0, 0($0)"),
    DEF_I_FORMAT(OP_SW, sw, "sw $0, 0($0)"),
    DEF_I_FORMAT(OP_SWR, swr, "swr $0, 0($0)"),

    DEF_R_FORMAT(0, nop, "nop"),
    DEF_R_FORMAT(0, noop, "noop"),

    { 0, NULL, NULL }
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
    struct test_inst *inst;
    struct parser parser;
    FILE *file;
    int ret = 0, comp;

    for (inst = asm_inst; inst->instname != NULL; inst++) {

        parser_init(&parser);
        file = fmemopen(inst->text, strlen(inst->text), "r");

        ret += test_assert_with_name(inst->instname, parser_load_asm(&parser, file) == 0);
        ret += test_assert_with_name(inst->instname, parser.text.len == 4);

        comp = be32_to_cpu(*(be32 *)parser.text.data);
        ret += test_assert_with_name(inst->instname, comp == inst->inst);

        parser_clear(&parser);
        fclose(file);
    }

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

