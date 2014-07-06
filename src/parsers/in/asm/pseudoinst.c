/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include "assembler_internal.h"
#include "mips.h"
#include "pseudoinst.h"

static void gen_li(struct assembler *a, const struct inst_generic *g, struct inst_reg *regs)
{
    uint32_t inst;

    inst = mips_create_i_format(OP_LUI, 0, regs[0].val, (regs[1].val) >> 16);
    add_word_to_seg(&a->text, inst);

    inst = mips_create_i_format(OP_ORI, regs[0].val, regs[0].val, (regs[1].val) & 0xFFFF);
    add_word_to_seg(&a->text, inst);
}

static void gen_la(struct assembler *a, const struct inst_generic *g, struct inst_reg *regs)
{
    uint32_t inst;

    inst = mips_create_i_format(OP_LUI, 0, regs[0].val, 0);
    create_marker(a, regs[1].ident, a->curlex.line, 16, 16, 0xFFFF, 0);
    add_word_to_seg(&a->text, inst);

    inst = mips_create_i_format(OP_ORI, regs[0].val, regs[0].val, 0);
    create_marker(a, regs[1].ident, a->curlex.line, 16, 0, 0xFFFF, 0);
    add_word_to_seg(&a->text, inst);
}

const struct pseudo_inst_desc inst_pseudo_ids[] = {
    { { .ident = "li", .reg_count = 2, { REG_REGISTER, REG_IMMEDIATE } }, gen_li },
    { { .ident = "la", .reg_count = 2, { REG_REGISTER, REG_ADDRESS } }, gen_la },
    { { NULL } }
};

