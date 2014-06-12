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
#include "asm.h"
#include "args.h"

const char *version_text =
    "cmips-" Q(CMIPS_VERSION_N) " - A command-line MIPS emulator written in C\n"
    "Copyright (C) 2014 Matt Kilgore\n"
    "Licensed under the GNU GPL, version 2\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRENTY, to the extent permitted by law\n"
    "\n"
;

struct arg {
    const char *lng;
    char shrt;
    const char *help_txt;

    int has_arg :1;
};

enum arg_list {
    ARG_EXTRA = -4,
    ARG_LNG = -3,
    ARG_ERR = -2,
    ARG_DONE = -1,
#define X(enu, id, arg, op, help_text) ARG_##enu,
# include "args.x"
#undef X
    ARG_LAST
};

static const struct arg cmips_args[] = {
#define X(enu, id, arg, op, help_text) [ARG_##enu] = { .lng = id, .shrt = op, .help_txt = help_text, .has_arg = arg },
# include "args.x"
#undef X
};

static char *argarg;

static int parser_argc, current_arg = 1;
static char **parser_argv;
static const char *shrt;
static int parsing_done;

enum arg_list arg_parser(void)
{
    const char *cur;
    if (current_arg == parser_argc)
        return ARG_DONE;

    cur = parser_argv[current_arg];

    if (parsing_done)
        goto parsing_done;

    if (cur[0] == '-' && !shrt) {
        if (*(cur + 1) && cur[1] == '-') {
            int i;

            if (!cur[2]) {
                parsing_done = 1;
                current_arg++;
                goto parsing_done;
            }


            for (i = 0; i < ARG_LAST; i++)
                if (strcmp(cmips_args[i].lng, cur + 2) == 0)
                    return i;

            printf("%s: unreconized argument '%s'\n", parser_argv[0], cur);
            current_arg++;
            return ARG_ERR;
        }

        shrt = cur + 1;
    }

    if (shrt) {
        int i;
        for (i = 0; i < ARG_LAST; i++) {
            if (cmips_args[i].shrt == *shrt) {
                shrt++;
                if (!*shrt) {
                    shrt = NULL;
                    current_arg++;
                }
                return i;
            }
        }

        printf("%s: unreconized argument '-%c'\n", parser_argv[0], *shrt);
        return ARG_ERR;
    }

parsing_done:

    argarg = parser_argv[current_arg];
    current_arg++;
    return ARG_EXTRA;
}

static void display_help_text(const char *prog)
{
    const struct arg *a;
    printf("Usage: %s [Flags] [Files] \n"
           "\n"
           "Files: Assembly source files to load on startup\n"
           "Flags:\n", prog);

    for (a = cmips_args; a->lng != NULL; a++) {
        printf("  ");
        if (a->shrt< 256)
            printf("-%c, ", a->shrt);
        else
            printf("     ");

        printf("--%-15s %s\n", a->lng, a->help_txt);
    }

    printf("See the manpage for more information\n");
}

void parse_args(int argc, char **argv, struct arg_state *s)
{
    enum arg_list ret;

    parser_argc = argc;
    parser_argv = argv;

    while ((ret = arg_parser()) != ARG_DONE) {
        switch (ret) {
        case ARG_help:
            display_help_text(argv[0]);
            exit(0);
            break;
        case ARG_version:
            printf(version_text);
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
        case ARG_EXTRA:
            asm_gen_from_file(&cmips_asm_gen, argarg);
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

