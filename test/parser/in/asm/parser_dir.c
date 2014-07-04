/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <string.h>

#include "test/test.h"
#include "parser.h"

struct test_dir {
    const char *name;
    const char *data;
    char *code;
    size_t len;
};

static struct test_dir dir_tests[] = {
    { ".data", "\x11\x22\x33\x44",
              ".data\n"
              ".word 0x11223344"
              , 4 },
    { ".word", "\x11\x22\x33\x44\x11\x22\x33\x44\x11\x11\x11\x11\xFF\xFF\xFF\xFF",
              ".data\n"
              ".word 0x11223344\n"
              ".word 0x11223344\n"
              ".word 0x11111111\n"
              ".word 0xFFFFFFFF\n"
              , 16 },
    { ".half", "\x11\x22\x33\x44\x11\x11\xFF\xFF",
               ".data\n"
               ".half 0x1122\n"
               ".half 0x3344\n"
               ".half 0x1111\n"
               ".half 0xFFFF\n"
               , 8 },
    { ".byte", "\x11\x22\x11\xFF",
               ".data\n"
               ".byte 0x11\n"
               ".byte 0x22\n"
               ".byte 0x11\n"
               ".byte 0xFF\n"
               , 4},
    { ".space", "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                ".data\n"
                ".space 10\n"
                ".space 6\n"
                , 16 },
    { ".asciiz", "Test test test\x00",
                ".data\n"
                ".asciiz \"Test test test\"\n"
                , 15 },
    { ".ascii", "Test test test",
                ".data\n"
                ".ascii \"Test test test\"\n"
                , 14 },
    { ".align", "\x11\x22\x33\x44\x00\x00\x00\x00\x11\x22\x33\x44",
                ".data\n"
                ".word 0x11223344\n"
                ".align 8\n"
                ".word 0x11223344\n"
                , 12 },
    { NULL, NULL, NULL }
};

int test_directives(void)
{
    int ret = 0;
    struct parser parser;
    FILE *file;
    struct test_dir *dir;

    for (dir = dir_tests; dir->name != NULL; dir++) {
        parser_init(&parser);
        file = fmemopen(dir->code, strlen(dir->code), "r");

        ret += test_assert_with_name(dir->name, parser_load_asm(&parser, file) == 0);
        ret += test_assert_with_name(dir->name, parser.data.len == dir->len);
        ret += test_assert_with_name(dir->name, memcmp(parser.data.data, dir->data, dir->len) == 0);

        fclose(file);
        parser_clear(&parser);
    }

    return ret;
}

