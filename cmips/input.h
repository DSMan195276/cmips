/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_INPUT_H
#define INCLUDE_INPUT_H

void exit_cmd(int argc, char **argv);
int run_script(const char *s_file);
void run_input_loop(void);

#endif
