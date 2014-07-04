/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test/test.h"
#include "test/parser/in/asm/parser_dir.h"
#include "test/parser/in/asm/parser_label.h"
#include "test/parser/in/asm/parser_inst.h"
#include "test/parser/in/asm/parser_args.h"
#include "parser.h"

int error_was_caught = 0;

static void err_catcher(const char *err, ...)
{
    error_was_caught = 1;
}

int test_inst(char *buf, const char *disp, uint32_t inst)
{
    struct parser parser;
    FILE *file;
    int ret = 0, comp;

    parser_init(&parser);
    parser.err_disp = err_catcher;

    file = fmemopen(buf, strlen(buf), "r");

    ret += test_assert_with_name(disp, parser_load_asm(&parser, file) == 0);
    ret += test_assert_with_name(disp, parser.text.len == 4);

    comp = be32_to_cpu(*(be32 *)parser.text.data);
    ret += test_assert_with_name(disp, comp == inst);

    fclose(file);
    parser_clear(&parser);

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { test_comp_asm, "asm parser" },
        { test_args, "asm cmd arg test" },
        { test_directives, "asm directives" },
        { test_label, "asm label test" },
    };

    ret = run_tests("Test asm parser", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}

