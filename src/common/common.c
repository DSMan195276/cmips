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
#include <string.h>
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

void dump_mem(const void *buf, size_t len, uint32_t base_addr)
{
    char strbuf[100], strbuf2[100] = { 0 };
    char *cur_b, *start, *to_print;
    const unsigned char *b = buf;
    int i = 0, j, skipping = 0;

    cur_b = strbuf;
    start = strbuf;

    for (; i < len; i += 16) {
        cur_b += sprintf(cur_b, "0x%08x  ", (i) + base_addr);
        for (j = i; j < i + 16; j++) {
            if (j < len)
                cur_b += sprintf(cur_b, "%02x ", (const unsigned int)b[j]);
            else
                cur_b += sprintf(cur_b, "   ");
            if (j - i == 7)
                *(cur_b++) = ' ';
        }

        cur_b += sprintf(cur_b, " |");
        for (j = i; j < i + 16 && j < len; j++)
            if (b[j] > 31 && b[j] <= 127)
                cur_b += sprintf(cur_b, "%c", b[j]);
            else
                *(cur_b++) = '.';
        cur_b += sprintf(cur_b, "|\n");

        to_print = start;

        if (start == strbuf)
            start = strbuf2;
        else
            start = strbuf;

        cur_b = start;

        /* The 12 magic number is just so we don't compare the printed address,
         * which is in '0x00000000' format at the beginning of the string */
        if (strcmp(strbuf + 12, strbuf2 + 12) != 0) {
            if (skipping == 1)
                printf("%s", start);
            else if (skipping == 2)
                printf("...\n");
            skipping = 0;
            printf("%s", to_print);
        } else if (skipping >= 1) {
            skipping = 2;
        } else {
            skipping = 1;
        }
    }
    if (skipping)
        printf("...\n%s", to_print);
}

