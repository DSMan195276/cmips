/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_MIPS_REGS_H
#define INCLUDE_MIPS_REGS_H
#include "common.h"

/* Structure holds all of the registers in the MIPS machine */
struct mips_regs {
    uint32_t regs[32];
    union {
        struct {
            uint32_t hi;
            uint32_t lo;
        };
        uint64_t hilo;
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


#endif
