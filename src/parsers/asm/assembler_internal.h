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
#include "rbtree.h"

enum reg_type {
    REG_REGISTER,
    REG_IMMEDIATE,
    REG_ADDRESS,
    REG_DEREF_REG
};

enum reg_place {
    REGP_RT,
    REGP_RD,
    REGP_RS,
    REGP_SA,
    REGP_IMMEDIATE,
    REGP_ADDRESS,
};

struct reg {
    uint32_t val;
};

struct inst_desc {
    const char *ident;
    enum inst_type format;
    int opcode;
    int func;
    int reg_count;
    enum reg_type rs[4];
    enum reg_place place[4];

    int addr_is_branch, addr_bits, addr_shift, addr_mask;
};

struct label_list {
    struct rbnode node;

    uint32_t addr;
    char ident[];
};

struct label_marker {
    struct rbnode node;

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

struct assembler {
    struct parser *gen;
    struct lexer lexer;

    enum asm_token tok;

    struct asm_segment text;
    struct asm_segment data;

    struct rbtree labels;
    struct rbtree markers;

    enum section cur_section;
};

#define expect_token(tok, val) \
    do { \
        if ((tok) != (val)) \
            return RET_UNEXPECTED; \
    } while (0)

void add_to_seg(struct asm_segment *seg, void *data, size_t len);
void add_word_to_seg(struct asm_segment *seg, uint32_t word);
void align_seg(struct asm_segment *seg, int alignment);
void create_marker(struct assembler *a, const char *ident, int bits, int shift, int mask, int is_branch);
const char *sect_to_str(enum section s);

#endif
