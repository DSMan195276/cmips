/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_MIPS_INST_H
#define INCLUDE_MIPS_INST_H

#include <stdint.h>

enum inst_type {
    I_FORMAT,
    J_FORMAT,
    R_FORMAT
};

enum inst_opcode {
#define X(op, val, fmt, func) OP_##op = val,
# include "opcode.x"
#undef X
};

enum inst_reg_type {
    REG_REGISTER,
    REG_IMMEDIATE,
    REG_ADDRESS,
    REG_DEREF_REG
};

enum inst_reg_place {
    REGP_RT,
    REGP_RD,
    REGP_RS,
    REGP_SA,
    REGP_IMMEDIATE,
    REGP_ADDRESS,
};

struct inst_reg {
    uint32_t val;
    char *ident;
};

struct inst_generic {
    const char *ident;
    int reg_count;
    enum inst_reg_type rs[4];
};

struct inst_desc {
    struct inst_generic g;

    enum inst_type format;
    int opcode;
    int func;
    enum inst_reg_place place[4];

    int addr_is_branch, addr_bits, addr_shift, addr_mask;
};

extern const struct inst_desc inst_ids[];

uint32_t inst_gen(const struct inst_desc *id, struct inst_reg *regs);
void mips_disp_inst(uint32_t inst);

extern const char *mips_opcode_names[64];

extern enum inst_type mips_opcode_to_type[64];

enum inst_function {
#define X(op, val, func) OP_FUNC_##op = val,
# include "mips/function.x"
#undef X
};

extern const char *mips_function_names[64];

#define INST_OPCODE(inst) ((inst) >> 26)

/* I_FORMAT */
#define INST_I_RS(inst)     (((inst) >> 21) & 0x1F)
#define INST_I_RT(inst)     (((inst) >> 16) & 0x1F)
#define INST_I_OFFSET(inst) ((inst) & 0xFFFF)

/* J_FORMAT */
#define INST_J_INDEX(inst)  ((inst) & 0x3FFFFFF)

/* R_FORMAT */
#define INST_R_RS(inst)     (((inst) >> 21) & 0x1F)
#define INST_R_RT(inst)     (((inst) >> 16) & 0x1F)
#define INST_R_RD(inst)     (((inst) >> 11) & 0x1F)
#define INST_R_SA(inst)     (((inst) >> 6)  & 0x1F)
#define INST_R_FUNC(inst)   ((inst) & 0x3F)

#define mips_create_i_format(op, rs, rt, offset) \
    ((uint32_t) (((op) << 26) + (((rs) & 0x1F) << 21) + (((rt) & 0x1F) << 16) + ((offset) & 0xFFFF)))

#define mips_create_j_format(op, jaddr) \
    ((uint32_t) (((op) << 26) + ((jaddr) & 0x3FFFFFFF)))

#define mips_create_r_format(op, rs, rt, rd, sa, func) \
    ((uint32_t) (((op) << 26) + (((rs) & 0x1F) << 21) + (((rt) & 0x1F) << 16) + (((rd) & 0x1F) << 11) + (((sa) & 0x1F) << 6) + ((func) & 0x3F)))

#endif
