/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>

#include "mips.h"
#include "lexer.h"
#include "asm.h"
#include "assembler_internal.h"
#include "mips/inst.h"

#define expect_token(tok, val) \
    do { \
        if ((tok) != (val)) \
            return RET_UNEXPECTED; \
    } while (0)

static void handle_inst(struct assembler *a, const struct inst_generic *inst, struct inst_reg *r)
{
    const struct inst_desc *desc = container_of(inst, const struct inst_desc, g);
    uint32_t op;
    int i;

    for (i = 0; i < 4; i++)
        if (r[i].ident)
            create_marker(a, r[i].ident, desc->addr_bits,
                desc->addr_shift, desc->addr_mask, desc->addr_is_branch == 2);

    op = inst_gen(desc, r);

    add_word_to_seg(&a->text, op);
}

enum internal_ret parse_instruction(struct assembler *a, const struct inst_generic *inst,
        void (*handler) (struct assembler *, const struct inst_generic *, struct inst_reg *))
{
    int i;
    struct inst_reg r[4];

    memset(r, 0, sizeof(struct inst_reg));

    for (i = 0; i < inst->reg_count; i++) {
        a->tok = yylex(&a->lexer);

        switch (inst->rs[i]) {
        case REG_REGISTER:
            expect_token(a->tok, TOK_REGISTER);

            r[i].val = a->lexer.val;
            break;
        case REG_IMMEDIATE:
            if (a->tok == TOK_INTEGER) {
                r[i].val = a->lexer.val;
            } else if (a->tok == TOK_IDENT) {
                r[i].val = 0;
                r[i].ident = strdup(a->lexer.ident);
            } else {
                return RET_UNEXPECTED;
            }
            break;
        case REG_ADDRESS:
            if (a->tok == TOK_INTEGER) {
                r[i].val = a->lexer.val >> 2;
            } else if (a->tok == TOK_IDENT) {
                r[i].val = 0;
                r[i].ident = strdup(a->lexer.ident);
            } else {
                return RET_UNEXPECTED;
            }
            break;
        case REG_DEREF_REG:
            expect_token(a->tok, TOK_INTEGER);

            r[i].val = a->lexer.val;

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_LPAREN);

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_REGISTER);

            r[i + 1].val = a->lexer.val;
            i++;

            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_RPAREN);

            break;
        }

        if (i != inst->reg_count - 1) {
            a->tok = yylex(&a->lexer);
            expect_token(a->tok, TOK_COMMA);
        }
    }

    (handler) (a, inst, r);

    for (i = 0; i < 4; i++)
        free(r[i].ident);

    return RET_CONTINUE;
}

enum internal_ret parse_command(struct assembler *a)
{
    const struct inst_desc *i;

    for (i = inst_ids; i->g.ident != NULL; i++)
        if (stringcasecmp(a->lexer.ident, i->g.ident) == 0)
            return parse_instruction(a, &i->g, handle_inst);

    return RET_UNEXPECTED;
}

