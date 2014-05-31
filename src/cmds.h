/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef SRC_CMDS_H
#define SRC_CMDS_H

struct cmips_cmd {
    const char *cmd_id;
    void (*cmd)(int argc, char **argv);

    const char *help;
};

extern struct cmips_cmd cmips_cmds[];

#endif
