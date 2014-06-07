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
#include <search.h>

#include "mips_emu.h"
#include "tokenizer.h"
#include "asm.h"

char *asm_escape_string(char *str)
{
    char *res;
    int cur = 0;
    int alloced = strlen(str) + 20;
    res = malloc(alloced);

    for (; *str; str++) {
        if (cur + 4 >= alloced) {
            alloced += 20;
            res = realloc(res, alloced);
        }

        switch (*str) {
        case '\t':
        case '\r':
        case '\n':
            res[cur++] = '\\';
            res[cur++] = 'x';
            sprintf(res + cur, "%02x", *str);
            cur += 2;
            break;
        default:
            res[cur++] = *str;
            break;
        }
    }

    if (cur + 1 >= alloced) {
        alloced += 1;
        res = realloc(res, alloced);
    }
    res[cur] = '\0';

    return res;
}

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
    ID_3_OPER_SPEC("add", OP_FUNC_ADD),
    ID_3_OPER_SPEC("addu", OP_FUNC_ADDU),
    ID_3_OPER_SPEC("sub", OP_FUNC_SUB),
    ID_3_OPER_SPEC("subu", OP_FUNC_SUBU),

    ID_3_OPER_I("addi", OP_ADDI),
    ID_3_OPER_I("addiu", OP_ADDIU),

    ID_J("j", OP_J),
    ID_J("jal", OP_JAL),

    { "nop", R_FORMAT, 0, 0, 0, { 0 }, { 0 } },
    { NULL }
};

struct key_list {
    struct key_list *next;
    uint32_t addr;
    char id[];
};

void asm_init(struct asm_gen *gen)
{
    memset(gen, 0, sizeof(struct asm_gen));
}

void asm_clear(struct asm_gen *gen)
{

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

int asm_gen_from_file(struct asm_gen *gen, const char *filename)
{
    struct token_list *list, *l;
    char *aline;
    FILE *file;

    file = fopen(filename, "r");
    if (file == NULL)
        return 1;

    list = tokenizer_run(file);
    fclose(file);

    file = fopen("output.s", "w");

    int line = 1, sline;

    for (l = list; l != NULL; l = l->next) {
        sline = 0;
        if (l->line > line) {
            sline = 1;
            do {
                fprintf(file, "\n");
                line++;
            } while (l->line > line);
        }

        switch (l->tok) {
        case TOK_LABEL:
            fprintf(file, "%s: ", l->ident);
            break;
        case TOK_DIRECTIVE:
            fprintf(file, ".%s ", asm_dir_types_str[l->val]);
            break;
        case TOK_REGISTER:
            fprintf(file, "$%s ", mips_reg_names_strs[l->val]);
            break;
        case TOK_IDENT:
            if (sline)
                fprintf(file, "\t");
            fprintf(file, "%s ", l->ident);
            break;
        case TOK_INTEGER:
            fprintf(file, "0x%hx ", l->val);
            break;
        case TOK_COMMA:
            fprintf(file, ", ");
            break;
        case TOK_LPAREN:
            fprintf(file, "( ");
            break;
        case TOK_RPAREN:
            fprintf(file, ") ");
            break;
        case TOK_SEMICOLON:
            fprintf(file, "; ");
            break;
        case TOK_QUOTE_STRING:
            aline = asm_escape_string(l->ident);
            fprintf(file, "\"%s\"", aline);
            free(aline);
            break;
        case TOK_EOF:
            break;
        }
        if (l->tok == TOK_LABEL)
            printf("Label: %d - %s\n", l->line, l->ident);
        else if (l->tok == TOK_DIRECTIVE)
            printf("Dir: %d - .%s\n", l->line, l->ident);
        else if (l->tok == TOK_REGISTER)
            printf("Register: %d - %d (%s)\n", l->line, l->val, mips_reg_names_strs[l->val]);
        else if (l->tok == TOK_IDENT)
            printf("Ident: %d - %s\n", l->line, l->ident);
        else if (l->tok == TOK_INTEGER)
            printf("Integer: %d - %d\n", l->line, l->val);
    }
    fclose(file);

    tokenizer_free_tokens(list);

    return 0;
}

