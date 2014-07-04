/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef TEST_TEST_PARSER_H
#define TEST_TEST_PARSER_H

#include <stdint.h>

extern int error_was_caught;
int test_inst(char *buf, const char *disp, uint32_t inst);

#endif
