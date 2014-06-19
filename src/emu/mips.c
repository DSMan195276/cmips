/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips.h"
#include "emu/mem.h"
#include "parser.h"
#include "emu.h"
#include "special.h"

static void op_jmp(struct mips_emu *emu, int addr)
{

}

static void op_jmp_load(struct mips_emu *emu, int addr)
{

}

static void op_addi(struct mips_emu *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = emu->r.regs[rs] + val;
}

static void nop_i(struct mips_emu *emu, int rs, int rt, int val)
{

}

enum inst_type mips_opcode_to_type[64] = {
#define X(op, code, fmt, func) [OP_##op] = fmt,
# include "mips/mips_emu_opcode.x"
#undef X
};

/* Note - This jmp table contains 4 types of function pointers
 *
 * Each instruction type has it's own format - We deconstruct the instruction
 * into it's various parts (rs, rt, ...) and then call the correct func with
 * those pieces. To handle this, the argument list is left unspecefied, and
 * mips_run_inst handles calling the functions with the correct parameters
 * based on the instruction format.
 */
static void (*op_jmp_table[64])() = {
#define X(op, code, fmt, func) [OP_##op] = func,
# include "mips/mips_emu_opcode.x"
#undef X
};

const char *mips_opcode_names[64] = {
#define X(op, val, fmt, func) [OP_##op] = #op,
# include "mips/mips_emu_opcode.x"
#undef X
};

const char *mips_function_names[64] = {
#define X(op, val, func) [OP_FUNC_##op] = #op,
# include "mips/mips_emu_function.x"
#undef X
};

void mips_disp_inst(uint32_t inst)
{
    enum inst_type t = mips_opcode_to_type[INST_OPCODE(inst)];
    printf("Inst: 0x%08x(%s)\n- ", inst, mips_opcode_names[INST_OPCODE(inst)]);
    if (t == R_FORMAT) {
        int rs = INST_R_RS(inst);
        int rt = INST_R_RT(inst);
        int rd = INST_R_RD(inst);
        int sa = INST_R_SA(inst);
        int func = INST_R_FUNC(inst);

        printf("R_FMT: rs: 0x%02x($%s), rt: 0x%02x($%s)\n         rd: 0x%02x($%s), sa: 0x%02x($%s), func: 0x%02x(%s)\n"
                , rs, mips_reg_names_strs[rs]
                , rt, mips_reg_names_strs[rt]
                , rd, mips_reg_names_strs[rd]
                , sa, mips_reg_names_strs[sa]
                , func, mips_function_names[func]);
    } else if (t == I_FORMAT) {
        int rs = INST_I_RS(inst);
        int rt = INST_I_RT(inst);
        int off = INST_I_OFFSET(inst);

        printf("I_FMT: rs: 0x%02x($%s), rt: 0x%02x($%s), off: 0x%04x(%d)\n"
                , rs, mips_reg_names_strs[rs]
                , rt, mips_reg_names_strs[rt]
                , off, off);
    } else if (t == J_FORMAT) {
        int addr = INST_J_INDEX(inst);

        printf("J_FMT: Jmp Addr: 0x%08x(%d) - Aligned: 0x%08x(%d)\n"
                , addr, addr
                , addr << 2, addr << 2);
    }
}

void mips_run_next_inst(struct mips_emu *emu)
{
    be32 inst;
    mem_read_from_addr(&emu->mem, emu->r.pc, sizeof(be32), &inst);
    emu->r.pc += 4;
    mips_run_inst(emu, be32_to_cpu(inst));
}

void mips_run_inst(struct mips_emu *emu, uint32_t inst)
{
    int op = INST_OPCODE(inst);
    void (*f)() = op_jmp_table[op];
    if (!f)
        return ;

    switch (mips_opcode_to_type[op]) {
    case R_FORMAT:
        {
            int rs = INST_R_RS(inst);
            int rt = INST_R_RT(inst);
            int rd = INST_R_RD(inst);
            int sa = INST_R_SA(inst);
            int func = INST_R_FUNC(inst);
            (f) (emu, rs, rt, rd, sa, func);
        }
        break;
    case I_FORMAT:
        {
            int rs = INST_I_RS(inst);
            int rt = INST_I_RT(inst);
            int off = INST_I_OFFSET(inst);
            (f) (emu, rs, rt, off);
        }
        break;
    case J_FORMAT:
        {
            int addr = INST_J_INDEX(inst);
            (f) (emu, addr);
        }
        break;
    }
}

void mips_run(struct mips_emu *emu)
{
    int i;
    for (i = 0; i < emu->mem.text.size && !emu->stop_prog; i+=4)
        mips_run_next_inst(emu);
}

void mips_reset_emu(struct mips_emu *emu)
{
    void *m;

    m = malloc(emu->backup_text.len);
    memcpy(m, emu->backup_text.data, emu->backup_text.len);
    mem_set_text(&emu->mem, emu->backup_text.addr, emu->backup_text.len, m);

    m = malloc(emu->backup_data.len);
    memcpy(m, emu->backup_data.data, emu->backup_data.len);
    mem_set_data(&emu->mem, emu->backup_data.addr, emu->backup_data.len, m);

    emu->r.pc = emu->backup_text.addr;
}

void mips_load_from_parser(struct mips_emu *emu, struct parser *parser)
{
    emu->backup_text.data = malloc(parser->text.len);
    memcpy(emu->backup_text.data, parser->text.data, parser->text.len);
    emu->backup_text.len = parser->text.len;
    emu->backup_text.alloced = parser->text.len;
    emu->backup_text.addr = parser->text.addr;

    emu->backup_data.data = malloc(parser->data.len);
    memcpy(emu->backup_data.data, parser->data.data, parser->data.len);
    emu->backup_data.len = parser->data.len;
    emu->backup_data.alloced = parser->data.len;
    emu->backup_data.addr = parser->data.addr;

    mips_reset_emu(emu);
}

int mips_load_from_file(struct mips_emu *emu, const char *filename)
{
    int ret = 0;
    struct parser parser;

    parser_init(&parser);

    if (parser_asm_file(&parser, filename)) {
        ret = 1;
        goto cleanup;
    }

    mips_load_from_parser(emu, &parser);

cleanup:
    parser_clear(&parser);
    return ret;
}

void mips_emu_init(struct mips_emu *emu)
{
    memset(emu, 0, sizeof(struct mips_emu));

    mem_prog_init(&emu->mem);
}

void mips_emu_clear(struct mips_emu *emu)
{
    mem_prog_clear(&emu->mem);
    free(emu->backup_text.data);
    free(emu->backup_data.data);
}

