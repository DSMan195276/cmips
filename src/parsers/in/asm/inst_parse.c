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
#include "pseudoinst.h"
#include "mips/inst.h"

static void handle_inst(struct assembler *a, const struct inst_generic *inst, struct inst_reg *r)
{
    const struct inst_desc *desc = container_of(inst, const struct inst_desc, g);
    uint32_t op;
    int i;

    for (i = 0; i < 4; i++)
        if (r[i].ident)
            create_marker(a, r[i].ident, a->curlex.line, desc->addr_bits,
                desc->addr_shift, desc->addr_mask, desc->addr_is_branch == 2);

    op = inst_gen(desc, r);

    add_word_to_seg(&a->text, op);
}

enum internal_ret parse_instruction(struct assembler *a, const struct inst_generic *inst,
        void (*handler) (struct assembler *, const struct inst_generic *, struct inst_reg *))
{
    int i;
    struct lexer_link *link = a->link;
    struct inst_reg r[4];

    memset(r, 0, sizeof(struct inst_reg) * 4);

    for (i = 0; i < inst->reg_count; i++) {
        link = get_next_link(a, link);

        switch (inst->rs[i]) {
        case REG_REGISTER:
            expect_token(a, link, TOK_REGISTER);

            r[i].val = link->lex.val;
            break;
        case REG_IMMEDIATE:
            if (link->tok == TOK_INTEGER) {
                r[i].val = link->lex.val;
            } else if (link->tok == TOK_IDENT) {
                r[i].val = 0;
                r[i].ident = strdup(link->lex.ident);
            } else {
                a->err_tok = link;
                return RET_UNEXPECTED;
            }
            break;
        case REG_ADDRESS:
            if (link->tok == TOK_INTEGER) {
                r[i].val = link->lex.val >> 2;
            } else if (link->tok == TOK_IDENT) {
                r[i].val = 0;
                r[i].ident = strdup(link->lex.ident);
            } else {
                a->err_tok = link;
                return RET_UNEXPECTED;
            }
            break;
        case REG_DEREF_REG:
            expect_token(a, link, TOK_INTEGER);

            r[i].val = link->lex.val;

            link = get_next_link(a, link);
            expect_token(a, link, TOK_LPAREN);

            link = get_next_link(a, link);
            expect_token(a, link, TOK_REGISTER);

            r[i + 1].val = link->lex.val;
            i++;

            link = get_next_link(a, link);
            expect_token(a, link, TOK_RPAREN);

            break;
        }

        if (i != inst->reg_count - 1) {
            link = get_next_link(a, link);
            expect_token(a, link, TOK_COMMA);
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
    const struct pseudo_inst_desc *p;

    struct lexer_link *link = a->link;

    for (i = inst_ids; i->g.ident != NULL; i++)
        if (stringcasecmp(link->lex.ident, i->g.ident) == 0)
            return parse_instruction(a, &i->g, handle_inst);

    for (p = inst_pseudo_ids; p->g.ident != NULL; p++)
        if (stringcasecmp(link->lex.ident, p->g.ident) == 0)
            return parse_instruction(a, &p->g, p->gen);

    a->err_tok = link;
    return RET_UNEXPECTED;
}

