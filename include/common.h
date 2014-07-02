/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 199309L

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

void dump_mem(const void *buf, size_t len, uint32_t base_addr);

typedef struct { uint32_t v; } be32;
typedef struct { uint32_t v; } le32;

typedef struct { uint16_t v; } be16;
typedef struct { uint16_t v; } le16;

static inline be32 le32_to_be32(le32 v)
{
    uint32_t val = (uint32_t)v.v;
    return (be32) {
        ((val & 0xFF000000) >> 24)
        | ((val & 0x00FF0000) >> 8)
        | ((val & 0x0000FF00) << 8)
        | ((val & 0x000000FF) << 24)
    };
}

static inline le32 be32_to_le32(be32 v)
{
    uint32_t val = (uint32_t)v.v;
    return (le32) {
        ((val & 0xFF000000) >> 24)
        | ((val & 0x00FF0000) >> 8)
        | ((val & 0x0000FF00) << 8)
        | ((val & 0x000000FF) << 24)
    };
}

static inline be16 le16_to_be16(le16 v)
{
    uint16_t val = (uint16_t)v.v;
    return (be16) {
        ((val & 0xFF00) >> 8)
        | ((val & 0x00FF) << 8)
    };
}

static inline le16 be16_to_le16(be16 v)
{
    uint16_t val = (uint16_t)v.v;
    return (le16) {
        ((val & 0xFF00) >> 8)
        | ((val & 0x00FF) << 8)
    };
}

#define le32_to_cpu(le32) ((uint32_t)(le32).v)
#define be32_to_cpu(be32) ((uint32_t)(be32_to_le32(be32).v))

#define cpu_to_le32(cpu) ((le32) { (cpu) })
#define cpu_to_be32(cpu) le32_to_be32((le32) { (cpu) })

#define le16_to_cpu(le16) ((uint16_t)(le16).v)
#define be16_to_cpu(be16) ((uint16_t)(be16_to_le16(be16).v))

#define cpu_to_le16(cpu) ((le16) { (cpu) })
#define cpu_to_be16(cpu) le16_to_be16((le16) { (cpu) })

#endif
