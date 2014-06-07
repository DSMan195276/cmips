/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

const char *asm_dir_types_str[] = {
#define X(id) #id,
# include "lex/tokenizer_lexer_dir.h"
#undef X
};

