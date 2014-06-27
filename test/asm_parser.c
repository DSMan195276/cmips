/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include "test.h"

int fake_test(void)
{
    TEST_ASSERT(1 == 1);
    TEST_ASSERT(1 == 1);
    TEST_ASSERT(1 == 1);
    return 0;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { fake_test, "Fake Test" }
    };

    ret = run_tests("Fake Test", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}

