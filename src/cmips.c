/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <stdarg.h>

#include "mips.h"
#include "emu.h"
#include "args.h"
#include "input.h"

struct emulator cmips_emu;
struct arg_state cmips_arg_state;

static void catch_errs(const char *err,va_list args)
{
    vprintf(err, args);
}

int main(int argc, char **argv)
{
    emulator_init(&cmips_emu);
    cmips_emu.err_disp = catch_errs;

    parse_args(argc, argv, &cmips_arg_state);

    if (!cmips_arg_state.quiet)
        printf("%s", version_text);

    if (cmips_arg_state.run)
        emulator_run(&cmips_emu);

    if (!cmips_arg_state.noinput)
        run_input_loop();

    emulator_clear(&cmips_emu);

    return 0;
}

