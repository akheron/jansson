/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <string.h>
#include "util.h"

static void file_not_found()
{
    json_t *json;
    json_error_t error;

    json = json_load_file("/path/to/nonexistent/file.json", 0, &error);
    if(json)
        fail("json_load_file returned non-NULL for a nonexistent file");
    if(error.line != -1)
        fail("json_load_file returned an invalid line number");
    if(strcmp(error.text, "unable to open /path/to/nonexistent/file.json: No such file or directory") != 0)
        fail("json_load_file returned an invalid error message");
}

static void reject_duplicates()
{
    json_error_t error;

    if(json_loads("{\"foo\": 1, \"foo\": 2}", JSON_REJECT_DUPLICATES, &error))
        fail("json_loads did not detect a duplicate key");
    check_error("duplicate object key near '\"foo\"'", "<string>", 1, 16, 16);
}

int main()
{
    file_not_found();
    reject_duplicates();

    return 0;
}
