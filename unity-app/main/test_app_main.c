/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "unity.h"

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}

void app_main(void)
{
    print_banner("Unity test runner");
    printf("Total test count: %d\n", unity_get_test_count());

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    unity_run_menu();
}