/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_SPECIAL_H
#define INCLUDE_SPECIAL_H

#include <stdint.h>

#include "mips_emu.h"

void op_special(struct mips_emu *emu, uint32_t inst);

#endif
