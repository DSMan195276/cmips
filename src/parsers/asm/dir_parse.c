/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>

#include "assembler_internal.h"
#include "lexer.h"
#include "dir_parse.h"

struct dir {
    const char *id;
    enum internal_ret (*func) (struct assembler *a);
    enum section required_sect;
};

static enum internal_ret dir_align(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_INTEGER);

    align_seg(&a->data, a->lexer.val);
    return RET_CONTINUE;
}

static enum internal_ret dir_ascii(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_QUOTE_STRING);

    add_to_seg(&a->data, a->lexer.ident, strlen(a->lexer.ident));
    return RET_CONTINUE;
}

static enum internal_ret dir_asciiz(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_QUOTE_STRING);

    add_to_seg(&a->data, a->lexer.ident, strlen(a->lexer.ident) + 1);
    return RET_CONTINUE;
}

static enum internal_ret dir_byte(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_INTEGER);

    add_to_seg(&a->data, &a->lexer.val, 1);
    return RET_CONTINUE;
}

static enum internal_ret dir_data(struct assembler *a)
{
    a->cur_section = SECT_DATA;
    return RET_CONTINUE;
}

static enum internal_ret dir_double(struct assembler *a)
{
    return RET_UNEXPECTED;
}

static enum internal_ret dir_extern(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_IDENT);

    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_INTEGER);

    return RET_CONTINUE;
}

static enum internal_ret dir_float(struct assembler *a)
{
    return RET_UNEXPECTED;
}

static enum internal_ret dir_globl(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_IDENT);

    return RET_CONTINUE;
}

static enum internal_ret dir_half(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_INTEGER);

    add_halfword_to_seg(&a->data, a->lexer.val);
    return RET_CONTINUE;
}

static enum internal_ret dir_kdata(struct assembler *a)
{
    return RET_CONTINUE;
}

static enum internal_ret dir_ktext(struct assembler *a)
{
    return RET_CONTINUE;
}

static enum internal_ret dir_space(struct assembler *a)
{
    a->tok = yylex(&a->lexer);
    expect_token(a->tok, TOK_INTEGER);

    printf("Space: %d\n", a->lexer.val);
    add_to_seg(&a->data, NULL, a->lexer.val);
    return RET_CONTINUE;
}

static enum internal_ret dir_text(struct assembler *a)
{
    a->cur_section = SECT_TEXT;
    return RET_CONTINUE;
}

static enum internal_ret dir_word(struct assembler *a)
{
    struct asm_segment *s;
    if (a->cur_section == SECT_TEXT)
        s = &a->text;
    else
        s = &a->data;

    a->tok = yylex(&a->lexer);
    if (a->tok == TOK_INTEGER) {
        add_word_to_seg(s, a->lexer.val);
    } else if (a->tok == TOK_IDENT) {
        add_word_to_seg(s, 0);
        create_marker(a, a->lexer.ident, 32, 0, 0xFFFFFFFF, 0);
    } else {
        return RET_UNEXPECTED;
    }

    return RET_CONTINUE;
}

static struct dir directives[] = {
#define X(id, enu, func, sect) { #id, func, sect },
# include "lexer_dir.x"
#undef X
    { 0 }
};

enum internal_ret parse_directive(struct assembler *a)
{
    struct dir *d;
    for (d = directives; d->id != NULL; d++) {
        if (stringcasecmp(d->id, a->lexer.ident) == 0) {
            if ((a->cur_section | d->required_sect) != d->required_sect) {
                printf(".%s is not allowed in the %s segment\n", d->id, sect_to_str(a->cur_section));
                return RET_ERR;
            }
            return (d->func) (a);
        }
    }

    return RET_UNEXPECTED;
}

