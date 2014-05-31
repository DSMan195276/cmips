/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mips_emu.h"
#include "asm.h"

enum reg_type {
    REG_REGISTER,
    REG_IMMEDIATE,
    REG_ADDRESS
};

enum reg_place {
    REG_RT,
    REG_RD,
    REG_RS,
    REG_SA,
    REG_IMM,
    REG_ADDR
};

struct reg {
    enum reg_type t;
    int val;
    int offset;
    int correct :1;
};

struct inst_desc {
    const char *ident;
    enum inst_type format;
    int opcode;
    int func;
    int reg_count;
    enum reg_type rs[4];
    enum reg_place place[4];
};

#define ID_3_OPER_SPEC(str, func) { str, R_FORMAT, OP_SPECIAL, (func), 3, { REG_REGISTER, REG_REGISTER, REG_REGISTER }, { REG_RD, REG_RS, REG_RT } }
#define ID_3_OPER_I(str, op) { str, I_FORMAT, op, 0, 3, { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, { REG_RT, REG_RS, REG_IMM } }
#define ID_J(str, op) { str, J_FORMAT, op, 0, 1, { REG_ADDRESS }, { REG_ADDR } }

static struct inst_desc ids[32] = {
    ID_3_OPER_I("addiu", OP_ADDIU),
    ID_3_OPER_I("addi", OP_ADDI),

    ID_3_OPER_SPEC("addu", OP_FUNC_ADDU),
    ID_3_OPER_SPEC("add", OP_FUNC_ADD),
    ID_3_OPER_SPEC("subu", OP_FUNC_SUBU),
    ID_3_OPER_SPEC("sub", OP_FUNC_SUB),

    ID_J("jal", OP_JAL),
    ID_J("j", OP_J),

    { "nop", R_FORMAT, 0, 0, 0, { 0 }, { 0 } },
    { NULL }
};

void asm_init(struct asm_gen *gen)
{
    memset(gen, 0, sizeof(struct asm_gen));
}

void asm_clear(struct asm_gen *gen)
{

}

static int startswith(const char *restrict prepend, const char *restrict line)
{
    int l1 = strlen(prepend);
    int l2 = strlen(line);

    return strncmp(line, prepend, (l1 > l2) ? l2 : l1);
}

static const char *get_reg_n(const char *reg, int *ret)
{
    int i = 0;

    for (; i < 32; i++) {
        if (startswith(reg, mips_reg_names_strs[i]) == 0) {
            *ret = i;
            return reg + strlen(mips_reg_names_strs[i]);
        }
    }

    return NULL;
}

static const char *parse_register(const char *line, struct reg *reg)
{
    const char *p = line;
    int n;

    if (*p != '$') {
        printf("ISSUE: %s\n", p);
        reg->correct = 0;
        return NULL;
    }
    p++;
    p = get_reg_n(p, &n);

    if (p == NULL) {
        reg->correct = 0;
        return NULL;
    }

    reg->val = n;
    return p;
}

static const char *parse_immediate(const char *line, struct reg *reg)
{
    char *end_ptr = NULL;
    reg->val = strtol(line, &end_ptr, 0);
    return end_ptr;
}

static const char *parse_address(const char *line, struct reg *reg)
{
    char *end_ptr = NULL;
    reg->val = strtol(line, &end_ptr, 0);
    return end_ptr;
}

static void parse_args(const char *line, struct reg *regs, int reg_count)
{
    const char *p = line;
    int i = 0;

    for (i = 0; i < reg_count; i++) {
        while (*p && (*p == '\t' || *p == ' '))
            p++;

        if (!*p)
            return;

        if (regs[i].t == REG_REGISTER)
            p = parse_register(p, regs + i);
        else if (regs[i].t == REG_IMMEDIATE)
            p = parse_immediate(p, regs + i);
        else if (regs[i].t == REG_ADDRESS)
            p = parse_address(p, regs + i);

        if (p == NULL) {
            regs[i].correct = 0;
            return ;
        }

        if (i < reg_count - 1) {
            while (*p && *p != ',')
                p++;
            if (!*p) {
                regs[i].correct = 0;
                return ;
            }
            p++;
        }
    }
}

static uint32_t gen_op(struct inst_desc *id, struct reg *regs)
{
    struct reg empty = { 0 };
    struct reg *rt = &empty;
    struct reg *rd = &empty;
    struct reg *rs = &empty;
    struct reg *sa = &empty;
    struct reg *imm = &empty;
    struct reg *addr = &empty;
    int i = 0;

    for (; i < id->reg_count; i++)
        if (id->place[i] == REG_RT)
            rt = regs + i;
        else if (id->place[i] == REG_RS)
            rs = regs + i;
        else if (id->place[i] == REG_RD)
            rd = regs + i;
        else if (id->place[i] == REG_SA)
            sa = regs + i;
        else if (id->place[i] == REG_IMM)
            imm = regs + i;
        else if (id->place[i] == REG_ADDR)
            addr = regs + i;

    if (id->format == I_FORMAT)
        return mips_create_i_format(id->opcode, rs->val, rt->val, imm->val);
    else if (id->format == R_FORMAT)
        return mips_create_r_format(id->opcode, rs->val, rt->val, rd->val, sa->val, id->func);
    else if (id->format == J_FORMAT)
        return mips_create_j_format(id->opcode, addr->val);

    return 0;
}

static void process_line(const char *line, uint32_t **buf, int *len)
{
    const char *p;
    int i;
    uint32_t op_code;
    struct inst_desc *id = ids;

    p = line;
    while ((*p == '\t' || *p == ' ') && *p)
        p++;

    for (; id->ident != NULL; id++) {
        if (startswith(id->ident, p) == 0) {
            struct reg rs[id->reg_count];
            for (i = 0; i < id->reg_count; i++)
                rs[i].t = id->rs[i];
            parse_args(p + strlen(id->ident), rs, id->reg_count);
            op_code = gen_op(id, rs);
            (*len)++;
            *buf = realloc(*buf, 4 * *len);
            (*buf)[*len - 1] = op_code;
            return ;
        }
    }

    return ;
}

int asm_gen_from_file(struct asm_gen *gen, const char *filename)
{
    uint32_t *c = NULL;
    char *line = NULL;
    FILE *file;
    int i = 0;
    size_t len = 0;

    file = fopen(filename, "r");
    if (file == NULL)
        return 1;

    while ((len = getline(&line, &len, file)) != -1)
        process_line(line, &c, &i);

    free(line);

    gen->text = (char *)c;
    gen->text_size = i * 4;

    fclose(file);
    return 0;
}

