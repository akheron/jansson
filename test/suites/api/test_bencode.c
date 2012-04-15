/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <string.h>
#include "util.h"

static void test_loads(void)
{
    json_error_t error;
    json_t *json;

    json = json_bencode_loads("d3:", 0, &error);
    if (json)
        fail("invalid bencode went undetected");
    check_error("partial string: 0/3", "<string>", -1, -1, 3);

    json = json_bencode_loads("li", 0, &error);
    if (json)
        fail("invalid bencode went undetected");
    check_error("unterminated integer", "<string>", -1, -1, 2);

    json = json_bencode_loads("li123e", 0, &error);
    if (json)
        fail("invalid bencode went undetected");
    check_error("unterminated list", "<string>", -1, -1, 6);

    json = json_bencode_loads("lx", 0, &error);
    if (json)
        fail("invalid bencode went undetected");
    check_error("invalid character: x", "<string>", -1, -1, 1);
}

static void run_tests(void)
{
	test_loads();
}
