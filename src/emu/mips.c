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
#include <signal.h>

#include "mips.h"
#include "emu/mem.h"
#include "parser.h"
#include "emu.h"
#include "common/buf.h"
#include "special.h"

static struct emulator *cur_run_emu = NULL;

static void sig_int_handler(int sigval)
{
    if (cur_run_emu)
        cur_run_emu->stop_prog = 1;
}

static void op_jmp(struct emulator *emu, int addr)
{
    uint32_t a = addr << 2;

    a |= (emu->r.pc & 0xC0000000);

    emu->r.pc = a;
}

static void op_jmp_load(struct emulator *emu, int addr)
{
    uint32_t old = emu->r.pc;
    uint32_t a = addr << 2;

    a |= (emu->r.pc & 0xC0000000);

    emu->r.pc = a;
    emu->r.regs[REG_RA] = old;
}

static void op_beq(struct emulator *emu, int rs, int rt, int val)
{
    if (emu->r.regs[rs] == emu->r.regs[rt])
        emu->r.pc += ((int16_t)val) << 2;
}

static void op_bne(struct emulator *emu, int rs, int rt, int val)
{
    if (emu->r.regs[rs] != emu->r.regs[rt])
        emu->r.pc += ((int16_t)val) << 2;
}

static void op_addi(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = emu->r.regs[rs] + (int16_t)val;
}

static void op_addiu(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (uint32_t)((uint32_t)emu->r.regs[rs] + (uint16_t)val);
}

static void op_andi(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (uint32_t)emu->r.regs[rs] & (uint32_t)val;
}

static void op_ori(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (uint32_t)emu->r.regs[rs] | (uint32_t)val;
}

static void op_xori(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (uint32_t)emu->r.regs[rs] ^ (uint32_t)val;
}

static void op_slti(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (int32_t)emu->r.regs[rs] < (int32_t)val;
}

static void op_sltiu(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (uint32_t)emu->r.regs[rs] < (uint32_t)val;
}

static void op_lui(struct emulator *emu, int rs, int rt, int val)
{
    emu->r.regs[rt] = (((uint32_t)val) << 16);
}

static void op_sw(struct emulator *emu, int rs, int rt, int val)
{
    be32 word;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    word = cpu_to_be32((uint32_t)emu->r.regs[rt]);
    mem_write_to_addr(&emu->mem, addr, sizeof(be32), &word);
}

static void op_sh(struct emulator *emu, int rs, int rt, int val)
{
    be16 half;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    half = cpu_to_be16((uint16_t)(emu->r.regs[rt] & 0xFFFF));
    mem_write_to_addr(&emu->mem, addr, sizeof(be16), &half);
}

static void op_sb(struct emulator *emu, int rs, int rt, int val)
{
    uint8_t byte;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    byte = (uint8_t)(emu->r.regs[rt] & 0xFF);
    mem_write_to_addr(&emu->mem, addr, sizeof(uint8_t), &byte);
}

static void op_lw(struct emulator *emu, int rs, int rt, int val)
{
    be32 word;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    mem_read_from_addr(&emu->mem, addr, sizeof(be32), &word);
    emu->r.regs[rt] = be32_to_cpu(word);
}

static void op_lh(struct emulator *emu, int rs, int rt, int val)
{
    be16 half;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    mem_read_from_addr(&emu->mem, addr, sizeof(be16), &half);
    emu->r.regs[rt] = (int32_t)((int16_t)be16_to_cpu(half));
}

static void op_lhu(struct emulator *emu, int rs, int rt, int val)
{
    be16 half;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    mem_read_from_addr(&emu->mem, addr, sizeof(be16), &half);
    emu->r.regs[rt] = (uint16_t)be16_to_cpu(half);
}

static void op_lb(struct emulator *emu, int rs, int rt, int val)
{
    int8_t byte;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    mem_read_from_addr(&emu->mem, addr, sizeof(int8_t), &byte);
    emu->r.regs[rt] = byte;
}

static void op_lbu(struct emulator *emu, int rs, int rt, int val)
{
    uint8_t byte;
    uint32_t addr = emu->r.regs[rs] + (int32_t)val;

    mem_read_from_addr(&emu->mem, addr, sizeof(uint8_t), &byte);
    emu->r.regs[rt] = byte;
}

static void nop_i(struct emulator *emu, int rs, int rt, int val)
{

}

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
# include "mips/opcode.x"
#undef X
};

void emulator_run_next_inst(struct emulator *emu)
{
    be32 inst;

    if (emu->stop_prog)
        return ;

    mem_read_from_addr(&emu->mem, emu->r.pc, sizeof(be32), &inst);
    emu->r.pc += 4;
    emulator_run_inst(emu, be32_to_cpu(inst));
}

void emulator_run_inst(struct emulator *emu, uint32_t inst)
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

void emulator_run(struct emulator *emu)
{
    cur_run_emu = emu;
    sigset(SIGINT, sig_int_handler);
    while (!emu->stop_prog)
        emulator_run_next_inst(emu);
    sigrelse(SIGINT);
}

void emulator_reset(struct emulator *emu)
{
    void *m;

    m = malloc(emu->backup_text.len);
    memcpy(m, emu->backup_text.data, emu->backup_text.len);
    mem_set_text(&emu->mem, emu->backup_text.addr, emu->backup_text.len, m);

    m = malloc(emu->backup_data.len);
    memcpy(m, emu->backup_data.data, emu->backup_data.len);
    mem_set_data(&emu->mem, emu->backup_data.addr, emu->backup_data.len, m);

    emu->r.pc = emu->backup_text.addr;
    emu->stop_prog = 0;
}

void emulator_load_from_parser(struct emulator *emu, struct parser *parser)
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

    emulator_reset(emu);
}

static int emulator_load_parser(struct emulator *emu, FILE *file, int (*parser_func) (struct parser *, FILE *))
{
    int ret = 0;
    struct parser parser;

    parser_init(&parser);

    parser.text.addr = 0x00400000;
    parser.data.addr = 0x00000000;
    parser.err_disp = emu->err_disp;

    if ((parser_func) (&parser, file)) {
        ret = 1;
        goto cleanup;
    }

    emulator_load_from_parser(emu, &parser);

cleanup:
    parser_clear(&parser);
    return ret;
}

int emulator_load_asm(struct emulator *emu, FILE *file)
{
    return emulator_load_parser(emu, file, parser_load_asm);
}

int emulator_load_from_file(struct emulator *emu, const char *filename)
{
    int ret = 0;
    FILE *file;
    int (*p) (struct parser *, FILE *);

    p = parser_get_correct_func(filename);

    file = fopen(filename, "r");
    if (file == NULL)
        return 1;

    ret = emulator_load_parser(emu, file, p);

    fclose(file);
    return ret;
}

void emulator_init(struct emulator *emu)
{
    memset(emu, 0, sizeof(struct emulator));

    buf_init(&emu->infd);
    emu->infd.fd = STDIN_FILENO;
    emu->outfd = STDOUT_FILENO;

    mem_prog_init(&emu->mem);
}

void emulator_clear(struct emulator *emu)
{
    mem_prog_clear(&emu->mem);
    buf_free(&emu->infd);
    free(emu->backup_text.data);
    free(emu->backup_data.data);
}

