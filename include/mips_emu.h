/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_MIPSEMU
#define INCLUDE_MIPSEMU
#include "common.h"

#include <stdint.h>

#include "mips_emu/mem.h"

/* Structure holds all of the registers in the MIPS machine */
struct mips_regs {
    uint32_t regs[32];
    union {
        struct {
            uint32_t hi;
            uint32_t lo;
        };
        uint64_t hilo;
        float hilofloat;
    };
    uint32_t pc;
};

enum mips_reg_names {
    REG_ZERO = 0, REG_AT, REG_V0, REG_V1, REG_A0,
    REG_A1, REG_A2, REG_A3, REG_T0, REG_T1,
    REG_T2, REG_T3, REG_T4, REG_T5, REG_T6,
    REG_T7, REG_S0, REG_S1, REG_S2, REG_S3,
    REG_S4, REG_S5, REG_S6, REG_S7, REG_T8,
    REG_T9, REG_K0, REG_K1, REG_GP, REG_SP,
    REG_FP, REG_RA
};

extern const char *mips_reg_names_strs[32];

void mips_dump_regs(struct mips_regs *);

/* This is an actual MIPS machine state. One of these should be created and
 * then sent to mips_emu_init() to be initalized. When you're done with it,
 * call mips_emu_clear to toss out any memory it may be holding on to. */
struct mips_emu {
    struct mips_regs r;
    struct mem_prog  mem;
};

void mips_emu_init(struct mips_emu *);
void mips_emu_clear(struct mips_emu *);

void mips_run_inst(struct mips_emu *, uint32_t inst);
void mips_run_next_inst(struct mips_emu *);

void mips_disp_inst(uint32_t inst);

enum inst_type {
    I_FORMAT,
    J_FORMAT,
    R_FORMAT
};

enum inst_opcode {
#define X(op, val, fmt, func) OP_##op = val,
# include "mips_emu/mips_emu_opcode.x"
#undef X
};

extern const char *mips_opcode_names[64];

extern enum inst_type mips_opcode_to_type[64];

enum inst_function {
#define X(op, val, func) OP_FUNC_##op = val,
# include "mips_emu/mips_emu_function.x"
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

static inline uint32_t mips_create_i_format(uint32_t op, uint32_t rs, uint32_t rt, uint32_t offset)
{
    return (op << 26) + ((rs & 0x1F) << 21) + ((rt & 0x1F) << 16) + (offset & 0xFFFF);
}

static inline uint32_t mips_create_j_format(uint32_t op, uint32_t j_addr)
{
    return (op << 26) + (j_addr & 0x3FFFFFF);
}

static inline uint32_t mips_create_r_format(uint32_t op, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t sa, uint32_t func)
{
    return (op << 26) + ((rs & 0x1F) << 21) + ((rt & 0x1F) << 16) + ((rd & 0x1F) << 11) + ((sa & 0x1F) << 6) + (func & 0x3F);
}

#endif
