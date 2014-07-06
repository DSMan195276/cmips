/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cmips.h"
#include "parser.h"
#include "emu.h"
#include "common/arg_parser.h"
#include "args.h"

const char *version_text =
    "cmips-" Q(CMIPS_VERSION_N) " - A command-line MIPS emulator written in C\n"
    "Copyright (C) 2014 Matt Kilgore\n"
    "Licensed under the GNU GPL, version 2\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRENTY, to the extent permitted by law\n"
    "\n"
;

enum arg_list {
    ARG_EXTRA = ARG_PARSER_EXTRA,
    ARG_LNG = ARG_PARSER_LNG,
    ARG_ERR = ARG_PARSER_ERR,
    ARG_DONE = ARG_PARSER_DONE,
#define X(enu, id, arg, op, help_text) ARG_##enu,
# include "args.x"
#undef X
    ARG_LAST
};

static const struct arg cmips_args[] = {
#define X(enu, id, arg, op, help_text) [ARG_##enu] = { .lng = id, .shrt = op, .help_txt = help_text, .has_arg = arg },
# include "args.x"
#undef X
    { 0 }
};

void parse_args(int argc, char **argv, struct arg_state *s)
{
    enum arg_list ret;

    while ((ret = arg_parser(argc, argv, cmips_args)) != ARG_DONE) {
        switch (ret) {
        case ARG_help:
            display_help_text(argv[0], cmips_args);
            exit(0);
            break;
        case ARG_version:
            printf("%s", version_text);
            exit(0);
            break;
        case ARG_quiet:
            s->quiet = 1;
            break;
        case ARG_run:
            s->run = 1;
            break;
        case ARG_noinput:
            s->noinput = 1;
            break;
        case ARG_script:
            s->cmd_script = argarg;
            break;
        case ARG_EXTRA:
            if (emulator_load_from_file(&cmips_emu, argarg)) {
                printf("Error assembling file.\n");
                s->quiet = 1;
                s->noinput = 1;
                return ;
            }
            break;
        default:
        case ARG_ERR:
            /* Error message already printed by getopt_long() */
            exit(0);
            break;
        }
    }

    return ;
}

