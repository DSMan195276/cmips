/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>

#include "mips_emu.h"
#include "lex/tokenizer_lexer.h"
#include "asm.h"
#include "assembler_internal.h"

#define ID_3_OPER_SPEC(str, funcn) { \
    .ident = str, \
    .format = R_FORMAT, \
    .opcode = OP_SPECIAL, \
    .func = (funcn), \
    .reg_count = 3, \
    .rs = { REG_REGISTER, REG_REGISTER, REG_REGISTER }, \
    .place = { REG_RD, REG_RS, REG_RT } \
}

#define ID_3_OPER_SPEC_NONE(str, funcn) { \
    .ident = str, \
    .format = R_FORMAT, \
    .opcode = OP_SPECIAL, \
    .func = (funcn), \
    .reg_count = 0, \
    .rs = { 0 }, \
    .place = { 0 } \
}

#define ID_3_OPER_JR(str, funcn) { \
    .ident = str, \
    .format = R_FORMAT, \
    .opcode = OP_SPECIAL, \
    .func = (funcn), \
    .reg_count = 1, \
    .rs = { REG_REGISTER }, \
    .place = { REG_RS } \
}

#define ID_3_OPER_I(str, op) { \
    .ident = str, \
    .format = I_FORMAT, \
    .opcode = op, \
    .func = 0, \
    .reg_count = 3, \
    .rs = { REG_REGISTER, REG_REGISTER, REG_IMMEDIATE }, \
    .place = { REG_RT, REG_RS, REG_IMM } \
}

#define ID_LUI(str, op) { \
    .ident = str, \
    .format = I_FORMAT, \
    .opcode = op, \
    .func = 0, \
    .reg_count = 2, \
    .rs = { REG_REGISTER, REG_IMMEDIATE }, \
    .place = { REG_RT, REG_IMM } \
}

#define ID_BRANCH(str, op) { \
    .ident = str, \
    .format = I_FORMAT, \
    .opcode = op, \
    .func = 0, \
    .reg_count = 3, \
    .rs = { REG_REGISTER, REG_REGISTER, REG_ADDRESS}, \
    .place = { REG_RS, REG_RT, REG_ADDR } \
}

#define ID_J(str, op) { \
    .ident = str, \
    .format = J_FORMAT, \
    .opcode = op, \
    .func = 0, \
    .reg_count = 1, \
    .rs = { REG_ADDRESS }, \
    .place = { REG_ADDR } \
}

#define ID_MEM(str, op) { \
    .ident = str, \
    .format = I_FORMAT, \
    .opcode = op, \
    .func = 0, \
    .reg_count = 3, \
    .rs = { REG_REGISTER, REG_DEREF_REG, REG_DEREF_REG }, \
    .place = { REG_RT, REG_IMM, REG_RS } \
}

static struct inst_desc ids[] = {
    ID_3_OPER_SPEC("sll", OP_FUNC_SLL),
    ID_3_OPER_SPEC("srl", OP_FUNC_SRL),
    ID_3_OPER_SPEC("sra", OP_FUNC_SRA),
    ID_3_OPER_SPEC("sllv", OP_FUNC_SLLV),
    ID_3_OPER_SPEC("srlv", OP_FUNC_SRLV),
    ID_3_OPER_SPEC("srav", OP_FUNC_SRAV),

    ID_3_OPER_JR("jr", OP_FUNC_JR),
    ID_3_OPER_JR("jalr", OP_FUNC_JALR),

    ID_3_OPER_SPEC("add", OP_FUNC_ADD),
    ID_3_OPER_SPEC("addu", OP_FUNC_ADDU),
    ID_3_OPER_SPEC("sub", OP_FUNC_SUB),
    ID_3_OPER_SPEC("subu", OP_FUNC_SUBU),
    ID_3_OPER_SPEC("and", OP_FUNC_AND),
    ID_3_OPER_SPEC("or", OP_FUNC_OR),
    ID_3_OPER_SPEC("xor", OP_FUNC_XOR),
    ID_3_OPER_SPEC("nor", OP_FUNC_NOR),
    ID_3_OPER_SPEC("slt", OP_FUNC_SLT),
    ID_3_OPER_SPEC("sltu", OP_FUNC_SLTU),

    ID_3_OPER_SPEC_NONE("syscall", OP_FUNC_SYSCALL),
    ID_3_OPER_SPEC_NONE("break", OP_FUNC_BREAK),

    ID_J("j", OP_J),
    ID_J("jal", OP_JAL),

    ID_BRANCH("beq", OP_BEQ),
    ID_BRANCH("bne", OP_BNE),

    ID_3_OPER_I("addi", OP_ADDI),
    ID_3_OPER_I("addiu", OP_ADDIU),
    ID_3_OPER_I("slti", OP_SLTI),
    ID_3_OPER_I("sltiu", OP_SLTIU),
    ID_3_OPER_I("andi", OP_ANDI),
    ID_3_OPER_I("ori", OP_ORI),
    ID_3_OPER_I("xori", OP_XORI),

    ID_LUI("lui", OP_LUI),

    ID_MEM("lb", OP_LB),
    ID_MEM("lh", OP_LH),
    ID_MEM("lwl", OP_LWL),
    ID_MEM("lw", OP_LW),
    ID_MEM("lbu", OP_LBU),
    ID_MEM("lhu", OP_LHU),
    ID_MEM("lwr", OP_LWR),
    ID_MEM("sb", OP_SB),
    ID_MEM("sh", OP_SH),
    ID_MEM("swl", OP_SWL),
    ID_MEM("sw", OP_SW),
    ID_MEM("swr", OP_SWR),

    { "nop", R_FORMAT, 0, 0, 0, { 0 }, { 0 } },
    { NULL }
};

static int stringcasecmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++)
        if ((*s1 | 32) != (*s2 | 32))
            return 1;

    if (*s1 || *s2)
        return 1;

    return 0;
}

static void add_to_seg(struct segment *seg, void *data, size_t len)
{
    if (seg->alloced - seg->len <= len) {
        size_t l = seg->len + len + 20;
        seg->data = realloc(seg->data, l);
        seg->alloced = l;
    }

    memcpy(seg->data + seg->len, data, len);
    seg->len += len;
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

#define expect_token(tok, val) \
    do { \
        if ((tok) != (val)) \
            return RET_UNEXPECTED; \
    } while (0)

static enum internal_ret parse_label(struct assembler *a)
{
    struct label_list *label;

    label = malloc(sizeof(struct label_list) + strlen(a->tokenizer.ident) + 1);

    if (a->cur_section == SECT_TEXT)
        label->addr = a->text.addr;
    else
        label->addr = a->data.addr;
    strcpy(label->id, a->tokenizer.ident);

    label->next = a->labels;
    a->labels = label;

    return RET_CONTINUE;
}

enum internal_ret parse_instruction(struct assembler *a, struct inst_desc *inst)
{
    int i;
    uint32_t op;
    struct reg r[4];

    printf("Parsing: %s, %d\n", inst->ident, inst->reg_count);

    memset(r, 0, sizeof(struct reg));

    for (i = 0; i < inst->reg_count; i++) {
        a->tok = yylex(&a->tokenizer);

        printf("Read token: %s, %s\n", asm_tok_types_str[a->tok], yytext);

        switch (inst->rs[i]) {
        case REG_REGISTER:
            expect_token(a->tok, TOK_REGISTER);

            r[i].val = a->tokenizer.val;
            break;
        case REG_IMMEDIATE:
            expect_token(a->tok, TOK_INTEGER);

            r[i].offset = a->tokenizer.val;
            break;
        case REG_ADDRESS:
            if (a->tok == TOK_INTEGER) {
                r[i].val = a->tokenizer.val;
            } else if (a->tok == TOK_IDENT) {

            } else {
                return RET_UNEXPECTED;
            }
            break;
        case REG_DEREF_REG:
            expect_token(a->tok, TOK_INTEGER);

            r[i].offset = a->tokenizer.val;

            a->tok = yylex(&a->tokenizer);
            expect_token(a->tok, TOK_LPAREN);

            a->tok = yylex(&a->tokenizer);
            expect_token(a->tok, TOK_REGISTER);

            r[i + 1].val = a->tokenizer.val;

            a->tok = yylex(&a->tokenizer);
            expect_token(a->tok, TOK_RPAREN);

            break;
        }

        if (i != inst->reg_count - 1) {
            a->tok = yylex(&a->tokenizer);
            expect_token(a->tok, TOK_COMMA);
        }
    }

    op = gen_op(inst, r);

    add_to_seg(&a->text, &op, sizeof(uint32_t));

    return RET_CONTINUE;
}

static enum internal_ret parse_command(struct assembler *a)
{
    struct inst_desc *i;

    for (i = ids; i->ident != NULL; i++)
        if (stringcasecmp(a->tokenizer.ident, i->ident) == 0)
            return parse_instruction(a, i);

    return RET_UNEXPECTED;
}

static enum internal_ret parse_directive(struct assembler *a)
{

    return RET_CONTINUE;
}

static void assembler_free(struct assembler *a)
{
    struct label_list *l, *ltmp;
    struct label_marker *m, *mtmp;

    free(a->tokenizer.ident);

    for (l = a->labels; l != NULL; l = ltmp) {
        ltmp = l->next;
        free(l);
    }

    for (m = a->markers; m != NULL; m = mtmp) {
        mtmp = m->next;
        free(m->label);
        free(m);
    }
}

int assemble_prog(struct asm_gen *gen, const char *filename)
{
    struct assembler a;
    FILE *file;
    int ret = 0;

    memset(&a, 0, sizeof(struct assembler));

    a.gen = gen;
    a.text.addr = gen->text_addr;
    a.data.addr = gen->data_addr;
    a.cur_section = SECT_TEXT;

    printf("Size: %d, %d\n", sizeof(ids), sizeof(ids)/sizeof(ids[0]));

    file = fopen(filename, "r");
    yyin = file;

#define call_parser(c) \
    switch (c(&a)) { \
    case RET_AGAIN: \
        goto again; \
    case RET_UNEXPECTED: \
        ret = 1; \
        goto exit; \
    default: \
        break; \
    }

    while ((a.tok = yylex(&a.tokenizer)) != TOK_EOF) {

        /* This label is here for the case that a token is read which an
         * internal function wants to pass off to the main parser. This label
         * jumps to the top of the loop and skips the yylex() read */
again:
        printf("Token: %s, %s\n", asm_tok_types_str[a.tok], yytext);

        switch (a.tok) {
        case TOK_LABEL:
            printf("Label: %s, 0x%08x\n", a.tokenizer.ident, a.text.addr);
            call_parser(parse_label);
            break;
        case TOK_DIRECTIVE:
            call_parser(parse_directive);
            break;
        case TOK_IDENT:
            call_parser(parse_command);
            break;
        case TOK_EOF:
            goto exit;

        default:
            ret = 1;
            goto exit;
        }

    }

exit:
    if (ret == 1)
        printf("Unexpected token on line %d: '%s'\n", a.tokenizer.line, yytext);

    assembler_free(&a);

    yyin = NULL;
    fclose(file);

    return ret;
}

