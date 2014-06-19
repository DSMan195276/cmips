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

struct ext_parser {
    const char *extension;
    int (*parser) (struct parser *, const char *);
} parsers[] = {
    { "s", parser_load_asm_file },
    { "asm", parser_load_asm_file },
    { NULL }
};

int parser_load_file(struct parser *parser, const char *filename)
{
    const char *e = NULL;
    const char *ext = filename;
    struct ext_parser *p;

    while (*ext) {
        if (*ext == '.')
            e = ext;
        ext++;
    }

    if (!e)
        return 1;

    e++;

    for (p = parsers; p->extension != NULL; p++)
        if (stringcasecmp(p->extension, e) == 0)
            return (p->parser) (parser, filename);

    return 1;
}

