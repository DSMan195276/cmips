/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <string.h>

#include "parser.h"

void parser_init(struct parser *parser)
{
    memset(parser, 0, sizeof(struct parser));
}

void parser_clear(struct parser *parser)
{
    free(parser->text.data);
    free(parser->data.data);
}

