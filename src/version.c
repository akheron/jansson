/*
 * Copyright (c) 2019 Sean Bright <sean.bright@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "jansson.h"

const char *jansson_version_str(void) { return JANSSON_VERSION; }

int jansson_version_cmp(int major, int minor, int micro) {
    if (JANSSON_MAJOR_VERSION != major)
        return JANSSON_MAJOR_VERSION < major ? -1 : 1;

    if (JANSSON_MINOR_VERSION != minor)
        return JANSSON_MINOR_VERSION < minor ? -1 : 1;

    if (JANSSON_MICRO_VERSION != micro)
        return JANSSON_MICRO_VERSION < micro ? -1 : 1;

    return 0;
}
