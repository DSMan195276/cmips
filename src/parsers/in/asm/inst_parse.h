/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_ASM_INST_PARSE_H
#define SRC_ASM_INST_PARSE_H

#include "mips/inst.h"

#include "assembler_internal.h"

enum internal_ret parse_command(struct assembler *a);
enum internal_ret parse_instruction(struct assembler *a, const struct inst_generic *inst,
        void (*handler) (struct assembler *, const struct inst_generic *, struct inst_reg *));

#endif
