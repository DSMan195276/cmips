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
#include "mips/inst.h"

enum syscall {
    SYSCALL_PRINT_INT = 1,
    SYSCALL_PRINT_STRING = 4,

    SYSCALL_READ_INT = 5,
    SYSCALL_READ_STRING = 8,

    SYSCALL_SBRK = 9,

    SYSCALL_EXIT = 10,

    SYSCALL_OPEN = 13,
    SYSCALL_READ_FD = 14,
    SYSCALL_WRITE_FD = 15,
    SYSCALL_CLOSE = 16,

    SYSCALL_EXIT2 = 17,

    SYSCALL_DELAY = 32,
    SYSCALL_PRINT_INT_HEX = 34,
    SYSCALL_PRINT_INT_BINARY = 35,
    SYSCALL_PRINT_INT_UNSIGNED = 36
};

#endif
