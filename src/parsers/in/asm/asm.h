/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H
#include "common.h"

#include <stdint.h>

#include "parser.h"

/* Must free result */
char *asm_escape_string(char *str);

void asm_disp_err(struct parser *p, const char *err, ...);

#endif
