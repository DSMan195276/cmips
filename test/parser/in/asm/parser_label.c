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

struct test_label {
    const char *name;
    const char *data;
    size_t data_size;
    const char *text;
    size_t text_size;
    char *code;
};

static struct test_label label_tests[] = {
    {
        "branch",
        "", 0,
        "\x00\x00\x00\x00\x14\x00\xFF\xFE", 8,
        "branch: nop\n"
        "bne $0, $0, branch\n"
    },
    {
        "branch2",
        "", 0,
        "\x14\x00\x00\x00\x00\x00\x00\x00", 8,
        "bne $0, $0, branch\n"
        "branch: nop\n"
    },
    {
        "jmp",
        "", 0,
        "\x00\x00\x00\x00\x08\x04\x00\x00", 8,
        "label: nop\n"
        "j label\n"
    },
    {
        "jmp2",
        "", 0,
        "\x08\x04\x00\x01\x00\x00\x00\x00", 8,
        "j label\n"
        "label: nop\n"
    },
    {
        "word",
        "\x00\x00\x00\x00\x00\x00\x00\x00", 8,
        "\x00\x00\x00\x00\x00\x00\x00\x04", 8,
        ".data\n"
        ".word 0x0\n"
        "label: .word 0x0\n"
        ".text\n"
        ".word 0x0\n"
        ".word label\n"
    },
    {
        "word2",
        "\x00\x00\x00\x00\x00\x10\x00\x04", 8,
        "\x00\x00\x00\x00\x00\x00\x00\x00", 8,
        ".data\n"
        ".word 0x0\n"
        ".word label\n"
        ".text\n"
        "nop\n"
        "label: nop\n"
    },
    { NULL, NULL, 0, NULL, 0, NULL }
};

int test_label(void)
{
    int ret = 0;
    struct parser parser;
    FILE *file;
    struct test_label *label;

    for (label = label_tests; label->name != NULL; label++) {
        parser_init(&parser);
        file = fmemopen(label->code, strlen(label->code), "r");

        parser.text.addr = 0x00100000;
        parser.data.addr = 0x00000000;

        ret += test_assert_with_name(label->name, parser_load_asm(&parser, file) == 0);
        ret += test_assert_with_name(label->name, parser.text.len == label->text_size);
        ret += test_assert_with_name(label->name, parser.data.len == label->data_size);
        ret += test_assert_with_name(label->name, memcmp(parser.text.data, label->text, label->text_size) == 0);
        ret += test_assert_with_name(label->name, memcmp(parser.data.data, label->data, label->data_size) == 0);

        fclose(file);
        parser_clear(&parser);
    }

    return ret;
}

