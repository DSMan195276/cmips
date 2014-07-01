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
    const uint32_t *insts;
    size_t inst_count;

    struct mips_regs r;
    struct mips_regs flags;
};

static struct test_cmd cmd_tests[] = {
    { "addi",
        (uint32_t []){ mips_create_i_format(OP_ADDI, 0, REG_T1, 30) }, 1,
        { { [REG_T1] = 30 } }, { { [REG_T1] = 1 } } },
    { "addi",
        (uint32_t []){ mips_create_i_format(OP_ADDI, 0, REG_T1, -1) }, 1,
        { { [REG_T1] = -1 } }, { { [REG_T1] = 1 } } },
    { "addiu",
        (uint32_t []){ mips_create_i_format(OP_ADDIU, 0, REG_T1, 0xFFF0) }, 1,
        { { [REG_T1] = 0xFFF0 } }, { { [REG_T1] = 1 } } },
    { "andi",
        (uint32_t []){ mips_create_i_format(OP_ADDI, 0, REG_T1, 0x0FF0),
                       mips_create_i_format(OP_ANDI, REG_T1, REG_T2, 0xFF00) }, 2,
        { { [REG_T2] = 0x0F00 } }, { { [REG_T2] = 1 } } },
    { "ori",
        (uint32_t []){ mips_create_i_format(OP_ADDI, 0, REG_T1, 0x0FF0),
                       mips_create_i_format(OP_ORI, REG_T1, REG_T2, 0xFF00) }, 2,
        { { [REG_T2] = 0xFFF0 } }, { { [REG_T2] = 1 } } },
    { "xori",
        (uint32_t []){ mips_create_i_format(OP_ADDI, 0, REG_T1, 0x0FF0),
                       mips_create_i_format(OP_XORI, REG_T1, REG_T2, 0xFF00) }, 2,
        { { [REG_T2] = 0xF0F0 } }, { { [REG_T2] = 1 } } },
    { "lui",
        (uint32_t []){ mips_create_i_format(OP_LUI, 0, REG_T1, 0xFFFFF) }, 1,
        { { [REG_T1] = 0xFFFF0000 } }, { { [REG_T1] = 1 } } },
    { NULL, NULL, 0, { { 0 } }, { { 0 } } }
};

int test_regs(void)
{
    int ret = 0, i, inst;
    struct emulator emu;

    for (i = 0; i < 32; i++) {
        inst = mips_create_i_format(OP_ADDI, 0, i, 20);

        emulator_init(&emu);

        emulator_run_inst(&emu, inst);
        ret += test_assert_with_name(mips_reg_names_strs[i], emu.r.regs[i] == 20);

        emulator_clear(&emu);
    }

    return ret;
}

int run_cmd_tests(void)
{
    int ret = 0, i;
    char buf[50];
    struct test_cmd *test;
    struct emulator emu;

    for (test = cmd_tests; test->name != NULL; test++) {
        emulator_init(&emu);

        for (i = 0; i < test->inst_count; i++)
            emulator_run_inst(&emu, test->insts[i]);

        for (i = 0; i < 32; i++) {
            if (test->flags.regs[i]) {
                sprintf(buf, "%s: %s", test->name, mips_reg_names_strs[i]);
                ret += test_assert_with_name(buf, emu.r.regs[i] == test->r.regs[i]);
            }
        }

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
