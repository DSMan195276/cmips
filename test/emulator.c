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
#include "emu.h"

struct test_cmd {
    const char *name;
    char *code;

    struct mips_regs r;
    struct mips_regs flags;

    const char *res_text;
    size_t res_text_len;
    const char *res_data;
    size_t res_data_len;
};

#define END_CODE "addi $v0, $0, 10\nsyscall\n"
#define END_TEXT "\x20\x02\x00\x0a\x00\x00\x00\x0C"
#define END_TEXT_LEN 8
#define END_REGS [REG_V0] = 10
#define END_FLAGS [REG_V0] = 1

static struct test_cmd cmd_tests[] = {
    { "addi", "addi $t1, $0, 30\n" END_CODE, { { [REG_T1] = 30, END_REGS } }, { { [REG_T1] = 1, END_FLAGS } }, NULL, 0, NULL, 0 },
    { "addi", "addi $t1, $0, -1\n" END_CODE, { { [REG_T1] = -1, END_REGS } }, { { [REG_T1] = 1, END_FLAGS } }, NULL, 0, NULL, 0 },
    { "addiu", "addiu $t1, $0, 0xFFF0\n" END_CODE, { { [REG_T1] = 0xFFF0, END_REGS } }, { { [REG_T1] = 1, END_FLAGS } }, NULL, 0, NULL, 0 },
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
    { "data",
        ".data\n"
        ".word 0x11223344\n"
        ".text\n"
        "nop\n"
        END_CODE,
        { { END_REGS } },
        { { END_FLAGS } },
        "\x00\x00\x00\x00" END_TEXT, 4 + END_TEXT_LEN,
        "\x11\x22\x33\x44", 4 },
    { "lui", "lui $t1, 0xFFFF\n" END_CODE, { { [REG_T1] = 0xFFFF0000, END_REGS } }, { { [REG_T1] = 1, END_FLAGS } }, NULL, 0, NULL,  },
    { NULL, NULL, { { 0 } }, { { 0 } }, NULL, 0, NULL, 0 }
};

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

        emulator_load_asm(&emu, file);
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

        if (test->res_text) {
            sprintf(buf, "%s: %s", test->name, "text");
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
            if (test_assert_with_name(buf, memcmp(emu.mem.data.block, test->res_data, test->res_data_len) == 0)) {
                ret++;
                printf("    Found:\n");
                dump_mem(emu.mem.data.block, emu.mem.data.size, emu.mem.data.addr);
                printf("    Expected:\n");
                dump_mem(test->res_data, test->res_data_len, emu.mem.data.addr);
            }
        }

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
