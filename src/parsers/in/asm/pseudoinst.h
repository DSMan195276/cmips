/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_ASM_PSEUDOINST_H
#define SRC_ASM_PSEUDOINST_H

#include "mips/inst.h"
#include "assembler_internal.h"

struct pseudo_inst_desc {
    struct inst_generic g;

    void (*gen) (struct assembler *, const struct inst_generic *, struct inst_reg *);
};

extern const struct pseudo_inst_desc inst_pseudo_ids[];

#endif
