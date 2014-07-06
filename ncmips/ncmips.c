/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "mips.h"
#include "emu.h"

struct emulator cmips_emu;

static void catch_errs(const char *err,va_list args)
{
    vprintf(err, args);
}

static void *run_emu(void *emu)
{
    emulator_run(emu);
    return NULL;
}

int main(int argc, char **argv)
{
    size_t size = 20;
    char buf[size + 1];
    int len;
    void *ret = NULL;
    struct timespec tim;

    memset(&tim, 0, sizeof(struct timespec));

    tim.tv_nsec = 100000000;

    int infd[2], outfd[2];
    pthread_t emu;

    emulator_init(&cmips_emu);
    cmips_emu.err_disp = catch_errs;
    emulator_load_from_file(&cmips_emu, "../asm/fibonacci.s");

    pipe(infd);
    pipe(outfd);

    cmips_emu.infd.fd = infd[0];
    cmips_emu.outfd = outfd[1];

    pthread_create(&emu, NULL, (void *(*)(void *))run_emu, &cmips_emu);

    write(infd[1], "45\n", 3);

    fcntl(outfd[0], F_SETFL, fcntl(outfd[0], F_GETFL) | O_NONBLOCK);

    while ((len = read(outfd[0], buf, size)) > 0 || !cmips_emu.stop_prog ) {
        if (len <= 0)
            len = 0;
        buf[len] = '\0';
        printf("%s", buf);
        nanosleep(&tim, NULL);
    }

    pthread_join(emu, &ret);

    close(infd[0]);
    close(infd[1]);
    close(outfd[0]);
    close(outfd[1]);

    emulator_clear(&cmips_emu);
    return 0;
}

