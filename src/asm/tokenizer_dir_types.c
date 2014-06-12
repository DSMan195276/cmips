/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

const char *asm_tok_types_str[] = {
#define X(id) #id,
# include "tokenizer_lexer.x"
#undef X
};

const char *asm_dir_types_str[] = {
#define X(id, func, sect) #id,
# include "tokenizer_lexer_dir.x"
#undef X
};

