/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_EMU_SYSCALL_H
#define SRC_EMU_SYSCALL_H

#include "emu.h"

void syscall_handler(struct emulator *emu, int rs, int rt, int rd, int sa);

#endif
