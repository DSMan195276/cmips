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

#include "rbtree.h"
#include "mips_emu.h"
#include "lex/tokenizer_lexer.h"
#include "asm.h"
#include "assembler_internal.h"
#include "inst_parse.h"
#include "dir_parse.h"
#include "label_parse.h"

void add_to_seg(struct asm_segment *seg, void *data, size_t len)
{
    if (seg->s.alloced - seg->s.len <= len) {
        size_t l = seg->s.len + len + 20;
        seg->s.data = realloc(seg->s.data, l);
        seg->s.alloced = l;
    }

    if (data != NULL)
        memcpy(seg->s.data + seg->s.len, data, len);
    else
        memset(seg->s.data + seg->s.len, 0, len);
    seg->s.len += len;
    seg->last_addr = seg->s.addr + seg->s.len;
}

const char *sect_to_str(enum section s)
{
    switch (s){
    case SECT_TEXT:
        return "text";
    case SECT_DATA:
        return "data";
    default:
        return NULL;
    }
}

static enum rbcomp label_cmp(const struct rbnode *c1, const struct rbnode *c2)
{
    const struct label_list *l1 = container_of(c1, struct label_list, node);
    const struct label_list *l2 = container_of(c2, struct label_list, node);
    int cmp = strcmp(l1->ident, l2->ident);

    if (cmp == 0)
        return RB_EQ;
    else if (cmp < 0)
        return RB_LT;
    else
        return RB_GT;
}

static enum rbcomp marker_cmp(const struct rbnode *c1, const struct rbnode *c2)
{
    const struct label_marker *m1 = container_of(c1, struct label_marker, node);
    const struct label_marker *m2 = container_of(c2, struct label_marker, node);
    int cmp = strcmp(m1->label, m2->label);

    if (cmp == 0)
        return RB_EQ;
    else if (cmp < 0)
        return RB_LT;
    else
        return RB_GT;
}

static void assembler_init(struct assembler *a)
{
    memset(a, 0, sizeof(struct assembler));
    a->labels.compare = label_cmp;
    a->markers.compare = marker_cmp;
}

static void assembler_free(struct assembler *a)
{
    struct rbnode *node = NULL;
    struct label_list *l;
    struct label_marker *m;

    free(a->tokenizer.ident);

    rb_foreach_postorder(&a->labels, node) {
        l = container_of(node, struct label_list, node);
        free(l);
    }

    rb_foreach_postorder(&a->markers, node) {
        m = container_of(node, struct label_marker, node);
        free(m);
    }
}

int assemble_prog(struct asm_gen *gen, const char *filename)
{
    struct assembler a;
    FILE *file;
    int ret = 0;

    assembler_init(&a);

    a.gen = gen;
    a.text.s.addr = gen->lowest_addr;
    a.data.s.addr = 0x10000000;
    a.cur_section = SECT_TEXT;

    file = fopen(filename, "r");
    yyin = file;

#define call_parser(c)   \
    switch (c(&a)) {     \
    case RET_AGAIN:      \
        goto again;      \
    case RET_UNEXPECTED: \
        ret = 1;         \
        goto exit;       \
    case RET_ERR:        \
        ret = 2;         \
        goto exit;       \
    default:             \
        break;           \
    }

    while ((a.tok = yylex(&a.tokenizer)) != TOK_EOF) {

        /* This label is here for the case that a token is read which an
         * internal function wants to pass off to the main parser. This label
         * jumps to the top of the loop and skips the yylex() read */
again:
        switch (a.tok) {
        case TOK_LABEL:
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
        printf("Unexpected characters on line %d: '%s'\n", a.tokenizer.line, yytext);

    gen->text.data = a.text.s.data;
    gen->text.len= a.text.s.len;;
    printf("Text segment (%d):\n", gen->text.len);
    dump_mem(gen->text.data, gen->text.len, a.text.s.addr);

    gen->data.data = a.data.s.data;
    gen->data.len = a.data.s.len;
    printf("Data segment (%d):\n", gen->data.len);
    dump_mem(gen->data.data, gen->data.len, a.data.s.addr);

    assembler_free(&a);
    yylex_destroy();

    yyin = NULL;
    fclose(file);

    return ret;
}

