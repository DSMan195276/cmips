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
#include <stdarg.h>
#include <stdint.h>

#include "test/test.h"
#include "mips.h"
#include "parser.h"
#include "emu.h"

int test_regs(void)
{
    int ret = 0, i, inst;
    struct emulator emu;

    for (i = 0; i < 32; i++) {
        inst = mips_create_i_format(OP_ADDI, 0, i, 20);

        emulator_init(&emu);

        emulator_run_inst(&emu, inst);
        if (test_assert_with_name(mips_reg_names_strs[i], emu.r.regs[i] == 20)) {
            ret++;
            printf("    Found: 0x%08x - Expected: 0x%08x\n", emu.r.regs[i], 20);
        }

        emulator_clear(&emu);
    }

    return ret;
}

static void disp_errors(const char *err, va_list args)
{
    vprintf(err, args);
}

struct test_cmd {
    const char *name;
    char *code;

    struct mips_regs r;
    struct mips_regs flags;

    /* Ending state of the text and data segments */
    const char *res_text;
    size_t res_text_len;

    const char *res_data;
    size_t res_data_len;
};

/* Code to end a program */
#define END_CODE "addi $v0, $0, 10\nsyscall\n"
/* The same code above, but in binary form */
#define END_TEXT "\x20\x02\x00\x0a\x00\x00\x00\x0C"
/* Length of the binary form */
#define END_TEXT_LEN 8
/* Ending settings from the end program code */
#define END_REGS [REG_V0] = 10
#define END_FLAGS [REG_V0] = 1

#define DEF_TEXT_SEG 0x00400000
#define DEF_DATA_SEG 0x00000000

static struct test_cmd cmd_tests[] = {
    { "addi",
        "addi $t1, $0, 30\n"
        END_CODE,
        { { [REG_T1] = 30, END_REGS } },
        { { [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "addi",
        "addi $t1, $0, -1\n"
        END_CODE,
        { { [REG_T1] = -1, END_REGS } },
        { { [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "addiu",
        "addiu $t1, $0, 0xFFF0\n"
        END_CODE,
        { { [REG_T1] = 0xFFF0, END_REGS } },
        { { [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "andi",
        "addi $t1, $0, 0x0FF0\n"
        "andi $t2, $t1, 0xFF00\n"
        END_CODE,
        { { [REG_T1] = 0x0FF0, [REG_T2] = 0x0F00, END_REGS } },
        { { [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "ori",
        "addi $t1, $0, 0x0FF0\n"
        "ori $t2, $t1, 0xFF00\n"
        END_CODE,
        { { [REG_T1] = 0x0FF0, [REG_T2] = 0xFFF0, END_REGS } },
        { { [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "xori",
        "addi $t1, $0, 0x0FF0\n"
        "xori $t2, $t1, 0xFF00\n"
        END_CODE,
        { { [REG_T1] = 0x0FF0, [REG_T2] = 0xF0F0, END_REGS } },
        { { [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "add",
        "addi $t0, $0, 30\n"
        "addi $t1, $0, 40\n"
        "add $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 30, [REG_T1] = 40, [REG_T2] = 70, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2 ] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "sub",
        "addi $t0, $0, 30\n"
        "addi $t1, $0, 40\n"
        "sub $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 30, [REG_T1] = 40, [REG_T2] = -10, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2 ] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "and",
        "addi $t0, $0, 0x0FF0\n"
        "addiu $t1, $0, 0xFF00\n"
        "and $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 0x0FF0, [REG_T1] = 0xFF00, [REG_T2] = 0x0F00, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2 ] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "or",
        "addi $t0, $0, 0x0FF0\n"
        "addiu $t1, $0, 0xFF00\n"
        "or $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 0x0FF0, [REG_T1] = 0xFF00, [REG_T2] = 0xFFF0, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2 ] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "xor",
        "addi $t0, $0, 0x0FF0\n"
        "addiu $t1, $0, 0xFF00\n"
        "xor $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 0x0FF0, [REG_T1] = 0xFF00, [REG_T2] = 0xF0F0, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2 ] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "nor",
        "addi $t0, $0, 0x0FF0\n"
        "addiu $t1, $0, 0xFF00\n"
        "nor $t2, $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = 0x0FF0, [REG_T1] = 0xFF00, [REG_T2] = 0xFFFF000F, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "lui",
        "addi $t1, $0, 20\n"
        "lui $t1, 0xFFFF\n"
         END_CODE,
        { { [REG_T1] = 0xFFFF0000, END_REGS } },
        { { [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "lw",
        ".data\n"
        "label: .word 0x11223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lw $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0x11223344, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\x11\x22\x33\x44", 4 },
    { "lh",
        ".data\n"
        "label: .word 0x11223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lh $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0x00001122, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\x11\x22\x33\x44", 4 },
    { "lh extend",
        ".data\n"
        "label: .word 0xF1223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lh $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0xFFFFF122, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\xF1\x22\x33\x44", 4 },
    { "lhu",
        ".data\n"
        "label: .word 0xF1223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lhu $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0x0000F122, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\xF1\x22\x33\x44", 4 },
    { "lb",
        ".data\n"
        "label: .word 0x11223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lb $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0x00000011, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\x11\x22\x33\x44", 4 },
    { "lb extend",
        ".data\n"
        "label: .word 0xF1223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lb $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0xFFFFFFF1, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\xF1\x22\x33\x44", 4 },
    { "lbu",
        ".data\n"
        "label: .word 0xF1223344\n"
        ".text\n"
        "addi $t1, $0, label\n"
        "lbu $t0, 0($t1)\n"
        END_CODE,
        { { [REG_T0] = 0x000000F1, [REG_T1] = DEF_DATA_SEG, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0,
        "\xF1\x22\x33\x44", 4 },
    { "sw",
        ".data\n"
        "label: .word 0x00000000\n"
        "value: .word 0x11223344\n"
        ".text\n"
        "addi $t0, $0, label\n"
        "addi $t1, $0, value\n"
        "lw $t2, 0($t1)\n"
        "sw $t2, 0($t0)\n"
        END_CODE,
        { { [REG_T0] = DEF_DATA_SEG, [REG_T1] = DEF_DATA_SEG + 4, [REG_T2] = 0x11223344, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0,
        "\x11\x22\x33\x44\x11\x22\x33\x44", 8 },
    { "sh",
        ".data\n"
        "label: .word 0x00000000\n"
        "value: .word 0x11223344\n"
        ".text\n"
        "addi $t0, $0, label\n"
        "addi $t1, $0, value\n"
        "lw $t2, 0($t1)\n"
        "sh $t2, 0($t0)\n"
        END_CODE,
        { { [REG_T0] = DEF_DATA_SEG, [REG_T1] = DEF_DATA_SEG + 4, [REG_T2] = 0x11223344, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0,
        "\x33\x44\x00\x00\x11\x22\x33\x44", 8 },
    { "sb",
        ".data\n"
        "label: .word 0x00000000\n"
        "value: .word 0x11223344\n"
        ".text\n"
        "addi $t0, $0, label\n"
        "addi $t1, $0, value\n"
        "lw $t2, 0($t1)\n"
        "sb $t2, 0($t0)\n"
        END_CODE,
        { { [REG_T0] = DEF_DATA_SEG, [REG_T1] = DEF_DATA_SEG + 4, [REG_T2] = 0x11223344, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0,
        "\x44\x00\x00\x00\x11\x22\x33\x44", 8 },
    { "mthi",
        "addi $t0, $0, 20\n"
        "mthi $t0\n"
        END_CODE,
        { { [REG_T0] = 20, END_REGS }, .hi = 20 },
        { { [REG_T0] = 1, END_FLAGS }, .hi = 1 },
        NULL, 0, NULL, 0 },
    { "mtlo",
        "addi $t0, $0, 20\n"
        "mtlo $t0\n"
        END_CODE,
        { { [REG_T0] = 20, END_REGS }, .lo = 20 },
        { { [REG_T0] = 1, END_FLAGS }, .lo = 1 },
        NULL, 0, NULL, 0 },
    { "mfhi",
        "addi $t0, $0, 20\n"
        "mthi $t0\n"
        "mfhi $t1\n"
        END_CODE,
        { { [REG_T0] = 20, [REG_T1] = 20, END_REGS }, .hi = 20 },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .hi = 1 },
        NULL, 0, NULL, 0 },
    { "mfhi",
        "addi $t0, $0, 20\n"
        "mtlo $t0\n"
        "mflo $t1\n"
        END_CODE,
        { { [REG_T0] = 20, [REG_T1] = 20, END_REGS }, .lo = 20 },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .lo = 1 },
        NULL, 0, NULL, 0 },
    { "sll",
        "addi $t0, $0, 0x00F0\n"
        "sll $t1, $t0, 4\n"
        END_CODE,
        { { [REG_T0] = 0x00F0, [REG_T1] = 0x0F00, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "srl",
        "lui $t0, 0xFFF0\n"
        "srl $t1, $t0, 4\n"
        END_CODE,
        { { [REG_T0] = 0xFFF00000, [REG_T1] = 0x0FFF0000, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "sra",
        "lui $t0, 0xFFF0\n"
        "sra $t1, $t0, 4\n"
        END_CODE,
        { { [REG_T0] = 0xFFF00000, [REG_T1] = 0xFFFF0000, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "sllv",
        "addi $t0, $0, 0x00F0\n"
        "addi $t2, $0, 4\n"
        "sllv $t1, $t0, $t2\n"
        END_CODE,
        { { [REG_T0] = 0x00F0, [REG_T1] = 0x0F00, [REG_T2] = 4, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "srlv",
        "lui $t0, 0xFFF0\n"
        "addi $t2, $0, 4\n"
        "srlv $t1, $t0, $t2\n"
        END_CODE,
        { { [REG_T0] = 0xFFF00000, [REG_T1] = 0x0FFF0000, [REG_T2] = 4, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "srav",
        "lui $t0, 0xFFF0\n"
        "addi $t2, $0, 4\n"
        "srav $t1, $t0, $t2\n"
        END_CODE,
        { { [REG_T0] = 0xFFF00000, [REG_T1] = 0xFFFF0000, [REG_T2] = 4, END_REGS } },
        { { [REG_T0] = 1, [REG_T1] = 1, [REG_T2] = 1, END_FLAGS } },
        NULL, 0, NULL, 0 },
    { "mult",
        "addi $t0, $0, -1\n"
        "add $t1, $0, $t0\n"
        "mult $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = -1, [REG_T1] = -1, END_REGS }, .hi = 0, .lo = 1 },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .hi = 1, .lo = 1 },
        NULL, 0, NULL, 0 },
    { "multu",
        "addi $t0, $0, -1\n"
        "add $t1, $0, $t0\n"
        "multu $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = -1, [REG_T1] = -1, END_REGS }, .hi = 0xFFFFFFFE, .lo = 1 },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .hi = 1, .lo = 1 },
        NULL, 0, NULL, 0 },
    { "div",
        "addi $t0, $0, -5\n"
        "addi $t1, $0, 2\n"
        "div $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = -5, [REG_T1] = 2, END_REGS }, .hi = 0xFFFFFFFF, .lo = 0xFFFFFFFE },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .hi = 1, .lo = 1 },
        NULL, 0, NULL, 0 },
    { "divu",
        "addi $t0, $0, -5\n"
        "addi $t1, $0, 2\n"
        "divu $t0, $t1\n"
        END_CODE,
        { { [REG_T0] = -5, [REG_T1] = 2, END_REGS }, .hi = 1, .lo = 0x7FFFFFFD },
        { { [REG_T0] = 1, [REG_T1] = 1, END_FLAGS }, .hi = 1, .lo = 1 },
        NULL, 0, NULL, 0 },
    { NULL, NULL, { { 0 } }, { { 0 } }, NULL, 0, NULL, 0 }
};


int run_cmd_tests(void)
{
    int ret = 0, i;
    char buf[50];
    FILE *file;
    struct test_cmd *test;
    struct emulator emu;

    for (test = cmd_tests; test->name != NULL; test++) {
        emulator_init(&emu);
        file = fmemopen(test->code, strlen(test->code), "r");

        emu.err_disp = disp_errors;

        if (test_assert_with_name(test->name, emulator_load_asm(&emu, file) == 0)) {
            ret++;
            goto cleanup;
        }

        emulator_run(&emu);

        for (i = 0; i < 32; i++) {
            if (test->flags.regs[i]) {
                sprintf(buf, "%s: %s", test->name, mips_reg_names_strs[i]);
                if (test_assert_with_name(buf, emu.r.regs[i] == test->r.regs[i])) {
                    ret++;
                    printf("   Found: 0x%08x - Expected: 0x%08x\n", emu.r.regs[i], test->r.regs[i]);
                }
            }
        }

        if (test->flags.hi) {
            sprintf(buf, "%s: hi", test->name);
            if (test_assert_with_name(buf, emu.r.hi == test->r.hi)) {
                ret++;
                printf("   Found: 0x%08x - Expected: 0x%08x\n", emu.r.hi, test->r.hi);
            }
        }

        if (test->flags.lo) {
            sprintf(buf, "%s: lo", test->name);
            if (test_assert_with_name(buf, emu.r.lo == test->r.lo)) {
                ret++;
                printf("   Found: 0x%08x - Expected: 0x%08x\n", emu.r.lo, test->r.lo);
            }
        }

        if (test->flags.pc) {
            sprintf(buf, "%s: pc", test->name);
            if (test_assert_with_name(buf, emu.r.pc == test->r.pc)) {
                ret++;
                printf("   Found: 0x%08x - Expected: 0x%08x\n", emu.r.pc, test->r.pc);
            }
        }

        if (test->res_text) {
            sprintf(buf, "%s: %s", test->name, "text");
            ret += test_assert_with_name(buf, emu.mem.text.size == test->res_text_len);
            if (test_assert_with_name(buf, memcmp(emu.mem.text.block, test->res_text, test->res_text_len) == 0)) {
                ret++;
                printf("    Found:\n");
                dump_mem(emu.mem.text.block, emu.mem.text.size, emu.mem.text.addr);
                printf("    Expected:\n");
                dump_mem(test->res_text, test->res_text_len, emu.mem.text.addr);
            }
        }


        if (test->res_data) {
            sprintf(buf, "%s: %s", test->name, "data");
            ret += test_assert_with_name(buf, emu.mem.data.size == test->res_data_len);
            if (test_assert_with_name(buf, memcmp(emu.mem.data.block, test->res_data, test->res_data_len) == 0)) {
                ret++;
                printf("    Found:\n");
                dump_mem(emu.mem.data.block, emu.mem.data.size, emu.mem.data.addr);
                printf("    Expected:\n");
                dump_mem(test->res_data, test->res_data_len, emu.mem.data.addr);
            }
        }

cleanup:
        fclose(file);
        emulator_clear(&emu);
    }

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { test_regs, "registers" },
        { run_cmd_tests, "command tests" },
    };

    ret = run_tests("emulator", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}
