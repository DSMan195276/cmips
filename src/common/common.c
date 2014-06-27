/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

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

int stringcasecmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++)
        if ((*s1 | 32) != (*s2 | 32))
            return 1;

    if (*s1 || *s2)
        return 1;

    return 0;
}

void dump_mem(void *buf, size_t len, uint32_t base_addr)
{
    unsigned char *b = buf;
    int i = 0, j;
    for (; i < len; i += 16) {
        printf("0x%08x  ", (i) + base_addr);
        for (j = i; j < i + 16; j++) {
            if (j < len)
                printf("%02x ", (unsigned char)b[j]);
            else
                printf("   ");
            if (j - i == 7)
                putchar(' ');
        }

        printf(" |");
        for (j = i; j < i + 16 && j < len; j++)
            if (b[j] > 31 && b[j] <= 127)
                printf("%c", b[j]);
            else
                putchar('.');
        printf("|\n");
    }
}

