/*
 * Copyright (c) 2019 Sean Bright <sean.bright@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "util.h"
#include <jansson.h>
#include <limits.h>
#include <string.h>

static void test_version_str(void) {
    if (strcmp(jansson_version_str(), JANSSON_VERSION)) {
        fail("jansson_version_str returned invalid version string");
    }
}

static void test_version_cmp() {
    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION,
                            JANSSON_MICRO_VERSION)) {
        fail("jansson_version_cmp equality check failed");
    }

    if (jansson_version_cmp(JANSSON_MAJOR_VERSION - 1, 0, 0) <= 0) {
        fail("jansson_version_cmp less than check failed");
    }

    if (JANSSON_MINOR_VERSION) {
        if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION - 1,
                                JANSSON_MICRO_VERSION) <= 0) {
            fail("jansson_version_cmp less than check failed");
        }
    }

    if (JANSSON_MICRO_VERSION) {
        if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION,
                                JANSSON_MICRO_VERSION - 1) <= 0) {
            fail("jansson_version_cmp less than check failed");
        }
    }

    if (jansson_version_cmp(JANSSON_MAJOR_VERSION + 1, JANSSON_MINOR_VERSION,
                            JANSSON_MICRO_VERSION) >= 0) {
        fail("jansson_version_cmp greater than check failed");
    }

    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION + 1,
                            JANSSON_MICRO_VERSION) >= 0) {
        fail("jansson_version_cmp greater than check failed");
    }

    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION,
                            JANSSON_MICRO_VERSION + 1) >= 0) {
        fail("jansson_version_cmp greater than check failed");
    }

    if (jansson_version_cmp(INT_MIN, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION) <= 0) {
        fail("jansson_version_cmp INT_MIN major check failed");
    }
    if (jansson_version_cmp(INT_MAX, JANSSON_MINOR_VERSION, JANSSON_MICRO_VERSION) >= 0) {
        fail("jansson_version_cmp INT_MAX major check failed");
    }
    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, INT_MIN, JANSSON_MICRO_VERSION) <= 0) {
        fail("jansson_version_cmp INT_MIN minor check failed");
    }
    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, INT_MAX, JANSSON_MICRO_VERSION) >= 0) {
        fail("jansson_version_cmp INT_MAX minor check failed");
    }
    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, INT_MIN) <= 0) {
        fail("jansson_version_cmp INT_MIN micro check failed");
    }
    if (jansson_version_cmp(JANSSON_MAJOR_VERSION, JANSSON_MINOR_VERSION, INT_MAX) >= 0) {
        fail("jansson_version_cmp INT_MAX micro check failed");
    }
}

static void run_tests() {
    test_version_str();
    test_version_cmp();
}
