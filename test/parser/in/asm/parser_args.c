/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>

#include "test/test.h"
#include "mips.h"
#include "test/parser.h"

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

