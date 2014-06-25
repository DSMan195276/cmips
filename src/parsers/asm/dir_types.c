/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include "lexer.h"

const char *asm_tok_types_str[] = {
#define X(id) #id,
# include "lexer.x"
#undef X
};

const char *asm_dir_types_str[] = {
#define X(id, enm, func, sect) [DIR_##enm] = #id,
# include "lexer_dir.x"
#undef X
};

