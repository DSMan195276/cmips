/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_ASM_ASSEMBLER_INTERNAL_H
#define SRC_ASM_ASSEMBLER_INTERNAL_H

#include <stdint.h>

#include "mips.h"
#include "lexer.h"
#include "parser.h"
#include "asm.h"
#include "common/rbtree.h"
#include "mips/inst.h"

struct label_list {
    struct rbnode node;

    uint32_t addr;
    char ident[];
};

struct label_marker {
    struct rbnode node;

    int line;

    uint32_t addr;
    uint32_t mask, shift, bits;

    unsigned int is_branch :1;

    char label[];
};

enum internal_ret {
    RET_AGAIN,
    RET_CONTINUE,
    RET_UNEXPECTED,
    RET_ERR
};

struct asm_segment {
    struct parser_segment s;
    uint32_t last_addr;
    uint32_t align_next;
};

enum section {
    SECT_TEXT = 1<<0,
    SECT_DATA = 1<<1,
};

struct lexer_link {
    struct lexer_link *next;
    struct lexer lex;
    enum asm_token tok;

    char *yytex;
};

struct assembler {
    struct parser *gen;
    struct lexer_link *link;

    struct lexer curlex;

    struct lexer_link *err_tok;

    struct asm_segment text;
    struct asm_segment data;

    struct rbtree labels;
    struct rbtree markers;

    enum section cur_section;
};

#define expect_token(a, link, val) \
    do { \
        if ((link->tok) != (val)) { \
            a->err_tok = link; \
            return RET_UNEXPECTED; \
        } \
    } while (0)

void add_to_seg(struct asm_segment *seg, void *data, size_t len);
void add_word_to_seg(struct asm_segment *seg, uint32_t word);
void add_halfword_to_seg(struct asm_segment *seg, uint16_t half);
void align_seg(struct asm_segment *seg, int alignment);
void create_marker(struct assembler *a, const char *ident, int line, int bits, int shift, int mask, int is_branch);
const char *sect_to_str(enum section s);

void clear_links(struct assembler *);
struct lexer_link *get_next_link(struct assembler *, struct lexer_link *);

#endif
