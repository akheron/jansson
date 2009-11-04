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
    "    \"a\":1,\n"                           \
    "    \"b\":2\n"                            \
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

int main(void)
{
    test_normal();
    test_indent();
    test_compact();
    test_compact_indent();

    return 0;
}
