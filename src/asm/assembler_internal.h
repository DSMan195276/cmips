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

#include "mips_emu.h"
#include "lex/tokenizer_lexer.h"
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
    REGP_ADDRRESS
};

struct reg {
    int val;
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

struct label_list {
    struct rbnode node;

    uint32_t addr;
    char ident[];
};

struct label_marker {
    struct rbnode node;

    uint32_t addr;
    unsigned int is_branch :1;
    unsigned int is_jmp    :1;

    char label[];
};

enum internal_ret {
    RET_AGAIN,
    RET_CONTINUE,
    RET_UNEXPECTED,
    RET_ERR
};

struct segment {
    char *data;
    size_t alloced;
    size_t len;
    uint32_t addr;
    uint32_t last_addr;
};

struct assembler {
    struct asm_gen *gen;
    struct tokenizer tokenizer;

    enum asm_token tok;

    struct segment text;
    struct segment data;

    struct rbtree labels;
    struct rbtree markers;

    enum {
        SECT_TEXT,
        SECT_DATA
    } cur_section;
};

#define expect_token(tok, val) \
    do { \
        if ((tok) != (val)) \
            return RET_UNEXPECTED; \
    } while (0)

void add_to_seg(struct segment *seg, void *data, size_t len);

#endif
