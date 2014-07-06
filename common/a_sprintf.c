/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <stdarg.h>

int a_sprintfv (char **buf, const char *format, va_list lst)
{
    size_t size = vsnprintf(NULL, 0, format, lst) + 1;

    *buf = malloc(size);

    if (!*buf)
        return -1;

    (*buf)[0] = '\0';

    size = vsnprintf(*buf, size, format, lst);
    return size;
}

int a_sprintf (char **buf, const char *format, ...)
{
    size_t ret;
    va_list lst;

    va_start(lst, format);
    ret = a_sprintfv(buf, format, lst);
    va_end(lst);

    return ret;
}

