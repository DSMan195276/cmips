/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200809

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* Inspired via the Linux-kernel macro 'container_of' */
#define container_of(ptr, type, member) \
    ((type *) ((char*)(ptr) - offsetof(type, member)))

#define QQ(x) #x
#define Q(x) QQ(x)

#define TP2(x, y) x ## y
#define TP(x, y) TP2(x, y)

extern int a_sprintf(char **buf, const char *format, ...);
extern int a_sprintfv(char **buf, const char *format, va_list list);

int stringcasecmp(const char *s1, const char *s2);

void dump_mem(void *buf, size_t len, uint32_t base_addr);

#endif
