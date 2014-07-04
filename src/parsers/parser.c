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

void parser_disp_err(struct parser *p, const char *err, ...)
{
    va_list args;
    /* We don't display errors unless we have a callback to call */
    if (!p->err_disp)
        return ;

    va_start(args, err);
    (p->err_disp) (err, args);
    va_end(args);
}

const static struct ext_parser {
    const char *extension;
    int (*parser) (struct parser *, FILE *);
} parsers[] = {
    { "s", parser_load_asm },
    { "asm", parser_load_asm },
    { NULL }
};

int (*parser_get_correct_func(const char *filename))
    (struct parser *, FILE *)
{
    const char *e = NULL;
    const char *ext = filename;
    const struct ext_parser *p;

    while (*ext) {
        if (*ext == '.')
            e = ext;
        ext++;
    }

    if (!e)
        return NULL;

    e++;

    for (p = parsers; p->extension != NULL; p++)
        if (stringcasecmp(p->extension, e) == 0)
            return p->parser;

    return NULL;
}

