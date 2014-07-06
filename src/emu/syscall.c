/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "mips.h"
#include "emu.h"
#include "syscall.h"

static void print_int(struct emulator *emu)
{
    dprintf(emu->outfd, "%d", emu->r.regs[REG_A0]);
}

static void print_int_hex(struct emulator *emu)
{
    dprintf(emu->outfd, "%08x", emu->r.regs[REG_A0]);
}

static void print_int_binary(struct emulator *emu)
{

}

static void print_int_unsigned(struct emulator *emu)
{
    dprintf(emu->outfd, "%u", emu->r.regs[REG_A0]);
}

static void print_string(struct emulator *emu)
{
    uint32_t addr;
    char byte;

    addr = emu->r.regs[REG_A0];
    while (mem_read_from_addr(&emu->mem, addr++, 1, &byte), byte != 0)
        write(emu->outfd, &byte, 1);
}

static void read_int(struct emulator *emu)
{
    char *line;
    int val, ret;
    fd_set writ;

    while (1) {
        buf_handle_input(&emu->infd);
        line = buf_read_line(&emu->infd);
        if (line) {
            ret = sscanf(line, "%d", &val);
            free(line);

            if (ret) {
                emu->r.regs[REG_V0] = val;
                return ;
            }
        }
        FD_ZERO(&writ);
        FD_SET(emu->infd.fd,  &writ);
        select(1, NULL, &writ, NULL, NULL);
    }
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
    [SYSCALL_READ_INT] = read_int,
    [SYSCALL_EXIT] = exit_prog,
    [SYSCALL_DELAY] = prog_delay,
    [SYSCALL_PRINT_INT_HEX] = print_int_hex,
    [SYSCALL_PRINT_INT_BINARY] = print_int_binary,
    [SYSCALL_PRINT_INT_UNSIGNED] = print_int_unsigned,
};

void syscall_handler(struct emulator *emu, int rs, int rt, int rd, int sa)
{
    void (*hand) (struct emulator *emu);

    hand = handlers[emu->r.regs[REG_V0]];
    if (hand)
        (hand) (emu);
}

