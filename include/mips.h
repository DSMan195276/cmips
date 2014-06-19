/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_MIPS
#define INCLUDE_MIPS
#include "common.h"

#include "mips/regs.h"

enum inst_type {
    I_FORMAT,
    J_FORMAT,
    R_FORMAT
};

enum inst_opcode {
#define X(op, val, fmt, func) OP_##op = val,
# include "mips/emu_opcode.x"
#undef X
};

void mips_disp_inst(uint32_t inst);

extern const char *mips_opcode_names[64];

extern enum inst_type mips_opcode_to_type[64];

enum inst_function {
#define X(op, val, func) OP_FUNC_##op = val,
# include "mips/emu_function.x"
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

enum syscall {
    SYSCALL_PRINT_INT = 1,
    SYSCALL_PRINT_STRING = 4,
    SYSCALL_READ_INT = 5,
    SYSCALL_EXIT = 10
};

#endif
