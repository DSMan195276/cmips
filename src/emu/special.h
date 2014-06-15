/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_MIPS_EMU_SPECIAL_H
#define SRC_MIPS_EMU_SPECIAL_H

#include <stdint.h>

#include "emu.h"

void op_special(struct mips_emu *emu, int rs, int rt, int rd, int sa, int func);

#endif
