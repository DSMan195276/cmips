/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <time.h>

#include "mips.h"
#include "emu.h"
#include "syscall.h"

static void print_int(struct emulator *emu)
{
    printf("%d", emu->r.regs[REG_A0]);
    fflush(stdout);
}

static void print_string(struct emulator *emu)
{
    uint32_t addr;
    char byte;

    addr = emu->r.regs[REG_A0];
    while (mem_read_from_addr(&emu->mem, addr++, 1, &byte), byte != 0)
        putchar(byte);
    fflush(stdout);
}

static void exit_prog(struct emulator *emu)
{
    emu->stop_prog = 1;
}

static void prog_delay(struct emulator *emu)
{
    struct timespec tim;

    tim = (struct timespec) { emu->r.regs[REG_A0] / 1000, (emu->r.regs[REG_A0] % 1000) * 1000000 };
    nanosleep(&tim, NULL);
}

void (*handlers[]) (struct emulator *emu) = {
    [SYSCALL_PRINT_INT] = print_int,
    [SYSCALL_PRINT_STRING] = print_string,
    [SYSCALL_EXIT] = exit_prog,
    [SYSCALL_DELAY] = prog_delay,
};

void syscall_handler(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    (handlers[emu->r.regs[REG_V0]]) (emu);
}

