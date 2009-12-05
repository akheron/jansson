/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <string.h>
#include "util.h"

static json_t *create_object()
{
    json_t *object;

    object = json_object();
    if(!object)
        fail("unable to create an object");

    if(json_object_set_new(object, "a", json_integer(1)) ||
       json_object_set_new(object, "b", json_integer(2)))
        fail("unable to set object values");

    return object;
}

static json_t *create_array()
{
    json_t *array;

    array = json_array();
    if(!array)
        fail("unable to create an array");

    if(json_array_append_new(array, json_integer(1)) ||
       json_array_append_new(array, json_integer(2)))
        fail("unable to append array values");

    return array;
}


#define NORMAL_OBJECT "{\"a\": 1, \"b\": 2}"
#define NORMAL_ARRAY  "[1, 2]"

static void test_normal()
{
    json_t *object;
    json_t *array;
    char *result;

    object = create_object();
    array = create_array();

    result = json_dumps(object, 0);
    if(strcmp(result, NORMAL_OBJECT) != 0)
        fail("unexpected encoded object");
    free(result);

    result = json_dumps(array, 0);
    if(strcmp(result, NORMAL_ARRAY) != 0)
        fail("unexpected encoded array");
    free(result);

    json_decref(object);
    json_decref(array);
}


#define INDENTED_OBJECT                         \
    "{\n"                                       \
    "    \"a\": 1,\n"                           \
    "    \"b\": 2\n"                            \
    "}"
#define INDENTED_ARRAY                          \
    "[\n"                                       \
    "    1,\n"                                  \
    "    2\n"                                   \
    "]"

static void test_indent()
{
    json_t *object;
    json_t *array;
    char *result;

    object = create_object();
    array = create_array();

    result = json_dumps(object, JSON_INDENT(4));
    if(strcmp(result, INDENTED_OBJECT) != 0)
        fail("unexpected encoded object");
    free(result);

    result = json_dumps(array, JSON_INDENT(4));
    if(strcmp(result, INDENTED_ARRAY) != 0)
        fail("unexpected encoded array");
    free(result);

    json_decref(object);
    json_decref(array);
}


#define COMPACT_OBJECT "{\"a\":1,\"b\":2}"
#define COMPACT_ARRAY  "[1,2]"

static void test_compact()
{
    json_t *object;
    json_t *array;
    char *result;

    object = create_object();
    array = create_array();

    result = json_dumps(object, JSON_COMPACT);
    if(strcmp(result, COMPACT_OBJECT) != 0)
        fail("unexpected encoded object");
    free(result);

    result = json_dumps(array, JSON_COMPACT);
    if(strcmp(result, COMPACT_ARRAY) != 0)
        fail("unexpected encoded array");
    free(result);

    json_decref(object);
    json_decref(array);
}


#define INDENTED_COMPACT_OBJECT                 \
    "{\n"                                       \
    "    \"a\":1,\n"                            \
    "    \"b\":2\n"                             \
    "}"
#define INDENTED_COMPACT_ARRAY                  \
    "[\n"                                       \
    "    1,\n"                                  \
    "    2\n"                                   \
    "]"

static void test_compact_indent()
{
    json_t *object;
    json_t *array;
    char *result;

    object = create_object();
    array = create_array();

    result = json_dumps(object, JSON_INDENT(4) | JSON_COMPACT);
    if(strcmp(result, INDENTED_COMPACT_OBJECT) != 0)
        fail("unexpected encoded object");
    free(result);

    result = json_dumps(array, JSON_INDENT(4) | JSON_COMPACT);
    if(strcmp(result, INDENTED_COMPACT_ARRAY) != 0)
        fail("unexpected encoded array");
    free(result);

    json_decref(object);
    json_decref(array);
}


static const char *test_ensure_ascii_data[][2] = {
    /*
    { "input", "output" }
    */

    /* ascii */
    { "foo", "foo" },

    /* BMP */
    { "\xc3\xa4 \xc3\xb6 \xc3\xa5", "\\u00e4 \\u00f6 \\u00e5" },
    { "foo \xc3\xa4\xc3\xa5", "foo \\u00e4\\u00e5" },
    { "\xc3\xa4\xc3\xa5 foo", "\\u00e4\\u00e5 foo" },
    { "\xc3\xa4 foo \xc3\xa5", "\\u00e4 foo \\u00e5" },

    /* non-BMP */
    { "clef g: \xf0\x9d\x84\x9e", "clef g: \\ud834\\udd1e" },
};

static void test_ensure_ascii()
{
    int i;
    int num_tests = sizeof(test_ensure_ascii_data) / sizeof(const char *) / 2;

    for(i = 0; i < num_tests; i++) {
        json_t *array, *string;
        const char *input, *output;
        char *result, *stripped;

        input = test_ensure_ascii_data[i][0];
        output = test_ensure_ascii_data[i][1];

        array = json_array();
        string = json_string(input);
        if(!array || !string)
            fail("unable to create json values");

        json_array_append(array, string);
        result = json_dumps(array, JSON_ENSURE_ASCII);

        /* strip leading [" and trailing "] */
        stripped = &result[2];
        stripped[strlen(stripped) - 2] = '\0';

        if(strcmp(stripped, output) != 0) {
            free(result);
            fail("the result of json_dumps is invalid");
        }
        free(result);
    }
}

int main(void)
{
    test_normal();
    test_indent();
    test_compact();
    test_compact_indent();
    test_ensure_ascii();

    return 0;
}
