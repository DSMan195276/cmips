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

int main()
{
    int ret;
    struct unit_test tests[] = {
        { test_regs, "registers" },
    };

    ret = run_tests("Test asm parser", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}
