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
#include "mips.h"
#include "lexer.h"
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

void add_word_to_seg(struct asm_segment *seg, uint32_t word)
{
    be32 val = cpu_to_be32(word);

    add_to_seg(seg, &val, sizeof(be32));
}

void align_seg(struct asm_segment *seg, int alignment)
{
    int new_last_addr = (seg->last_addr + alignment - 1) & ~(alignment - 1);
    int len = new_last_addr - seg->last_addr;

    if (len > 0)
        add_to_seg(seg, NULL, len);
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

void create_marker(struct assembler *a, const char *ident, int bits, int shift, int mask, int is_branch)
{
    struct label_marker *m;
    size_t len = strlen(ident);

    m = malloc(sizeof(struct label_marker) + len + 1);
    memset(m, 0, sizeof(struct label_marker) + len + 1);
    strcpy(m->label, ident);

    if (a->cur_section == SECT_TEXT)
        m->addr = a->text.last_addr;
    else
        m->addr = a->data.last_addr;

    m->is_branch = is_branch;
    m->bits = bits;
    m->shift = shift;
    m->mask = mask;

    rb_insert(&a->markers, &m->node);
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
    struct label_list *l;
    struct label_marker *m;

    free(a->lexer.ident);

    rb_foreach_postorder(&a->labels, l, struct label_list, node)
        free(l);

    rb_foreach_postorder(&a->markers, m, struct label_marker, node) {
        free(m);
    }

    free(a->text.s.data);
    free(a->data.s.data);
}

static struct label_list *find_label(struct assembler *a, const char *label)
{
    size_t len;
    struct label_list *l, *lab;

    len = strlen(label);

    lab = malloc(sizeof(struct label_list) + len + 1);
    memset(lab, 0, sizeof(struct label_list) + len + 1);
    strcpy(lab->ident, label);

    l = container_of(rb_search(&a->labels, &lab->node), struct label_list, node);

    free(lab);

    return l;
}

static void handle_markers(struct assembler *a)
{
    uint32_t addr;

    be32 *inst = NULL;

    struct label_marker *m;
    struct label_list *l;

    rb_foreach_inorder(&a->markers, m, struct label_marker, node) {
        l = find_label(a, m->label);

        addr = l->addr;
        addr &= m->mask;

        if (m->is_branch)
            addr -= m->addr + 4;

        addr >>= m->shift;

        if (a->text.s.addr <= m->addr && a->text.last_addr >= m->addr)
            inst = (be32 *) (((char *)a->text.s.data) + m->addr - a->text.s.addr);
        else if (a->data.s.addr >= m->addr && a->data.s.addr + a->data.s.len >= m->addr)
            inst = (be32 *) (((char *)a->data.s.data) + m->addr - a->data.s.addr);
        else
            inst = NULL;

        *inst = cpu_to_be32((be32_to_cpu(*inst) & ~((1 << (m->bits + 1)) - 1))
                | (addr & ((1 << (m->bits + 1)) - 1)));
    }
}

int assemble_prog(struct parser *gen, const char *filename)
{
    struct assembler a;
    FILE *file;
    int ret = 0;

    assembler_init(&a);

    a.gen = gen;
    a.text.s = gen->text;
    a.data.s = gen->data;
    a.text.last_addr = a.text.s.addr;
    a.data.last_addr = a.data.s.addr;
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

    while ((a.tok = yylex(&a.lexer)) != TOK_EOF) {

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

    handle_markers(&a);

exit:
    if (ret == 1)
        printf("Unexpected characters on line %d: '%s'\n", a.lexer.line, yytext);

    gen->text.data = a.text.s.data;
    gen->text.len = a.text.s.len;;
    a.text.s.data = NULL;

    gen->data.data = a.data.s.data;
    gen->data.len = a.data.s.len;
    a.data.s.data = NULL;

    assembler_free(&a);
    yylex_destroy();

    yyin = NULL;
    fclose(file);

    return ret;
}

