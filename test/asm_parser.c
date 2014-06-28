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

int test_inst(char *buf, const char *disp, uint32_t inst)
{
    struct parser parser;
    FILE *file;
    int ret = 0, comp;

    parser_init(&parser);
    file = fmemopen(buf, strlen(buf), "r");

    ret += test_assert_with_name(disp, parser_load_asm(&parser, file) == 0);
    ret += test_assert_with_name(disp, parser.text.len == 4);

    comp = be32_to_cpu(*(be32 *)parser.text.data);
    ret += test_assert_with_name(disp, comp == inst);

    fclose(file);
    parser_clear(&parser);

    return ret;
}

int test_comp_asm(void)
{
    struct test_inst *inst;
    int ret = 0;

    for (inst = asm_inst; inst->instname != NULL; inst++)
        ret += test_inst(inst->text, inst->instname, inst->inst);

    return ret;
}

int test_args(void)
{
    char buf[50];
    int i, ret = 0, inst;

    for (i = 0; i < 32; i++) {
        inst = mips_create_r_format(OP_SPECIAL, i, i, i, 0, OP_FUNC_ADD);
        sprintf(buf, "add $%d, $%d, $%d", i, i, i);
        ret += test_inst(buf, buf, inst);
    }

    for (i = 0; i < 32; i++) {
        inst = mips_create_r_format(OP_SPECIAL, i, i, i, 0, OP_FUNC_ADD);
        sprintf(buf, "add $%s, $%s, $%s", mips_reg_names_strs[i], mips_reg_names_strs[i], mips_reg_names_strs[i]);
        ret += test_inst(buf, buf, inst);
    }

    for (i = 0; i < 32; i++) {
        inst = mips_create_r_format(OP_SPECIAL, 0, 0, 0, i, OP_FUNC_SLLV);
        sprintf(buf, "sllv $0, $0, $%s", mips_reg_names_strs[i]);
        ret += test_inst(buf, buf, inst);
    }

    inst = mips_create_j_format(OP_J, 0x03FFFFFF);
    sprintf(buf, "j 0x0FFFFFFC");
    ret += test_inst(buf, buf, inst);

    inst = mips_create_i_format(OP_ADDI, 0, 0, 0xFFFF);
    sprintf(buf, "addi $0, $0, -1");
    ret += test_inst(buf, buf, inst);

    return ret;
}

struct test_dir {
    const char *name;
    const char *data;
    char *code;
    size_t len;
};

static struct test_dir dir_tests[] = {
    { ".data", "\x11\x22\x33\x44",
              ".data\n"
              ".word 0x11223344"
              , 4 },
    { ".word", "\x11\x22\x33\x44\x11\x22\x33\x44\x11\x11\x11\x11\xFF\xFF\xFF\xFF",
              ".data\n"
              ".word 0x11223344\n"
              ".word 0x11223344\n"
              ".word 0x11111111\n"
              ".word 0xFFFFFFFF\n"
              , 16 },
    { ".half", "\x11\x22\x33\x44\x11\x11\xFF\xFF",
               ".data\n"
               ".half 0x1122\n"
               ".half 0x3344\n"
               ".half 0x1111\n"
               ".half 0xFFFF\n"
               , 8 },
    { ".byte", "\x11\x22\x11\xFF",
               ".data\n"
               ".byte 0x11\n"
               ".byte 0x22\n"
               ".byte 0x11\n"
               ".byte 0xFF\n"
               , 4},
    { ".space", "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                ".data\n"
                ".space 10\n"
                ".space 6\n"
                , 16 },
    { ".asciiz", "Test test test\x00",
                ".data\n"
                ".asciiz \"Test test test\"\n"
                , 15 },
    { ".ascii", "Test test test",
                ".data\n"
                ".ascii \"Test test test\"\n"
                , 14 },
    { ".align", "\x11\x22\x33\x44\x00\x00\x00\x00\x11\x22\x33\x44",
                ".data\n"
                ".word 0x11223344\n"
                ".align 8\n"
                ".word 0x11223344\n"
                , 12 },
    { NULL, NULL, NULL }
};

int test_directives(void)
{
    int ret = 0;
    struct parser parser;
    FILE *file;
    struct test_dir *dir;

    for (dir = dir_tests; dir->name != NULL; dir++) {
        parser_init(&parser);
        file = fmemopen(dir->code, strlen(dir->code), "r");

        ret += test_assert_with_name(dir->name, parser_load_asm(&parser, file) == 0);
        ret += test_assert_with_name(dir->name, parser.data.len == dir->len);
        ret += test_assert_with_name(dir->name, memcmp(parser.data.data, dir->data, dir->len) == 0);

        fclose(file);
        parser_clear(&parser);
    }

    return ret;
}

struct test_label {
    const char *name;
    const char *data;
    size_t data_size;
    const char *text;
    size_t text_size;
    char *code;
};

static struct test_label label_tests[] = {
    {
        "branch",
        "", 0,
        "\x00\x00\x00\x00\x14\x00\xFF\xFE", 8,
        "branch: nop\n"
        "bne $0, $0, branch\n"
    },
    {
        "branch2",
        "", 0,
        "\x14\x00\x00\x00\x00\x00\x00\x00", 8,
        "bne $0, $0, branch\n"
        "branch: nop\n"
    },
    {
        "jmp",
        "", 0,
        "\x00\x00\x00\x00\x08\x04\x00\x00", 8,
        "label: nop\n"
        "j label\n"
    },
    {
        "jmp2",
        "", 0,
        "\x08\x04\x00\x01\x00\x00\x00\x00", 8,
        "j label\n"
        "label: nop\n"
    },
    {
        "word",
        "\x00\x00\x00\x00\x00\x00\x00\x00", 8,
        "\x00\x00\x00\x00\x00\x00\x00\x04", 8,
        ".data\n"
        ".word 0x0\n"
        "label: .word 0x0\n"
        ".text\n"
        ".word 0x0\n"
        ".word label\n"
    },
    {
        "word2",
        "\x00\x00\x00\x00\x00\x10\x00\x04", 8,
        "\x00\x00\x00\x00\x00\x00\x00\x00", 8,
        ".data\n"
        ".word 0x0\n"
        ".word label\n"
        ".text\n"
        "nop\n"
        "label: nop\n"
    },
    { NULL, NULL, 0, NULL, 0, NULL }
};

int test_label(void)
{
    int ret = 0;
    struct parser parser;
    FILE *file;
    struct test_label *label;

    for (label = label_tests; label->name != NULL; label++) {
        parser_init(&parser);
        file = fmemopen(label->code, strlen(label->code), "r");

        parser.text.addr = 0x00100000;
        parser.data.addr = 0x00000000;

        ret += test_assert_with_name(label->name, parser_load_asm(&parser, file) == 0);
        ret += test_assert_with_name(label->name, parser.text.len == label->text_size);
        ret += test_assert_with_name(label->name, parser.data.len == label->data_size);
        ret += test_assert_with_name(label->name, memcmp(parser.text.data, label->text, label->text_size) == 0);
        ret += test_assert_with_name(label->name, memcmp(parser.data.data, label->data, label->data_size) == 0);

        fclose(file);
        parser_clear(&parser);
    }

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { test_comp_asm, "asm parser" },
        { test_args, "asm cmd arg test" },
        { test_directives, "asm directives" },
        { test_label, "asm label test" },
    };

    ret = run_tests("Test asm parser", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}

