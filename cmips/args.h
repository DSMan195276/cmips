/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_ARGS_H
#define SRC_ARGS_H

struct arg_state {
    int quiet :1;
    int run :1;
    int noinput :1;

    const char *cmd_script;
};

void parse_args(int argc, char **argv, struct arg_state *);

extern const char *version_text;

#endif
