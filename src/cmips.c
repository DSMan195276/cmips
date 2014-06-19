/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>

#include "mips.h"
#include "emu.h"
#include "args.h"
#include "input.h"

struct mips_emu cmips_emu;
struct arg_state cmips_arg_state;

int main(int argc, char **argv)
{
    mips_emu_init(&cmips_emu);

    parse_args(argc, argv, &cmips_arg_state);

    if (!cmips_arg_state.quiet)
        printf("%s", version_text);

    if (cmips_arg_state.run)
        mips_run(&cmips_emu);

    if (!cmips_arg_state.noinput)
        run_input_loop();

    mips_emu_clear(&cmips_emu);

    return 0;
}

