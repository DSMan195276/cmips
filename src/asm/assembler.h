/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_ASM_ASSEMBLER_H
#define SRC_ASM_ASSEMBLER_H

#include "asm.h"

int assemble_prog(struct asm_gen *gen, const char *filename);

#endif
