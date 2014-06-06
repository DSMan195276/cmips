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
#include "asm/asm.h"
#include "args/args.h"

const char *version_text =
    "cmips-" Q(CMIPS_VERSION_N) " - A command-line MIPS emulator written in C\n"
    "Copyright (C) 2014 Matt Kilgore\n"
    "Licensed under the GNU GPL, version 2\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRENTY, to the extent permitted by law\n"
    "\n"
;

/*
 * help (noarg)
 * version (noarg)
 * quiet (noarg)
 * run (noarg)
 * load (arg)
 */
static const char *shortopts = "hvqrl:";

/* These are the getopt return values for various long-options. They match the
 * short-option if there is one, else they are simply a unique number greater
 * then 255 (Greater then any char value) */
#define OPT_HELP     'h'
#define OPT_VERSION  'v'
#define OPT_QUIET    'q'
#define OPT_RUN      'r'
#define OPT_LOAD     'l'

/* The definition of the long-options. Every option has a long-option, not all
 * long-options have a short-option. */
static const struct option longopts[] = {
#define X(id, arg, shrt, op, help_text) { id, arg, NULL, op },
# include "args_x.h"
#undef X
    {0}
};

static const char *help_text[] = {
#define X(id, arg, shrt, op, help_text) help_text,
# include "args_x.h"
#undef X
};

static void display_help_text(const char *prog)
{
    int i;
    printf("Usage: %s [Flags] \n"
           "\n"
           "Flags:\n", prog);

    for (i = 0; longopts[i].name != NULL; i++) {
        printf("  ");
        if (longopts[i].val < 256)
            printf("-%c, ", longopts[i].val);
        else
            printf("     ");

        printf("--%-15s %s\n", longopts[i].name, help_text[i]);
    }

    printf("See the manpage for more information\n");
}

void parse_args(int argc, char **argv, struct arg_state *s)
{
    int opt;
    int long_index = 0;

    while ((opt = getopt_long(argc, argv, shortopts, longopts, &long_index)) != -1) {
        switch (opt) {
        case OPT_HELP:
            display_help_text(argv[0]);
            exit(0);
            break;
        case OPT_VERSION:
            printf(version_text);
            exit(0);
            break;
        case OPT_QUIET:
            s->quiet = 1;
            break;
        case OPT_RUN:
            s->run = 1;
            break;
        case OPT_LOAD:
            asm_gen_from_file(&cmips_asm_gen, optarg);
            break;
        default:
        case '?':
            /* Error message already printed by getopt_long() */
            exit(0);
            break;
        }
    }

    return ;
}

