/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2010-2011 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include <jansson.h>
#include <stdio.h>
#include "util.h"

int main()
{
    json_t *value;
    int i;
    json_error_t error;

    /*
     * Simple, valid json_pack cases
     */

    /* true */
    value = json_pack_ex(&error, 0, "b", 1);
    if(!json_is_true(value))
        fail("json_pack boolean failed");
    if(value->refcount != (ssize_t)-1)
        fail("json_pack boolean refcount failed");
    json_decref(value);

    /* false */
    value = json_pack_ex(&error, 0, "b", 0);
    if(!json_is_false(value))
        fail("json_pack boolean failed");
    if(value->refcount != (ssize_t)-1)
        fail("json_pack boolean refcount failed");
    json_decref(value);

    /* null */
    value = json_pack_ex(&error, 0, "n");
    if(!json_is_null(value))
        fail("json_pack null failed");
    if(value->refcount != (ssize_t)-1)
        fail("json_pack null refcount failed");
    json_decref(value);

    /* integer */
    value = json_pack_ex(&error, 0, "i", 1);
    if(!json_is_integer(value) || json_integer_value(value) != 1)
        fail("json_pack integer failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack integer refcount failed");
    json_decref(value);


    /* real */
    value = json_pack_ex(&error, 0, "f", 1.0);
    if(!json_is_real(value) || json_real_value(value) != 1.0)
        fail("json_pack real failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack real refcount failed");
    json_decref(value);

    /* string */
    value = json_pack_ex(&error, 0, "s", "test");
    if(!json_is_string(value) || strcmp("test", json_string_value(value)))
        fail("json_pack string failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack string refcount failed");
    json_decref(value);

    /* empty object */
    value = json_pack_ex(&error, 0, "{}", 1.0);
    if(!json_is_object(value) || json_object_size(value) != 0)
        fail("json_pack empty object failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack empty object refcount failed");
    json_decref(value);

    /* empty list */
    value = json_pack_ex(&error, 0, "[]", 1.0);
    if(!json_is_array(value) || json_array_size(value) != 0)
        fail("json_pack empty list failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack empty list failed");
    json_decref(value);

    /* non-incref'd object */
    value = json_pack_ex(&error, 0, "o", json_integer(1));
    if(!json_is_integer(value) || json_integer_value(value) != 1)
        fail("json_pack object failed");
    if(value->refcount != (ssize_t)1)
        fail("json_pack integer refcount failed");
    json_decref(value);

    /* incref'd object */
    value = json_pack_ex(&error, 0, "O", json_integer(1));
    if(!json_is_integer(value) || json_integer_value(value) != 1)
        fail("json_pack object failed");
    if(value->refcount != (ssize_t)2)
        fail("json_pack integer refcount failed");
    json_decref(value);
    json_decref(value);

    /* simple object */
    value = json_pack_ex(&error, 0, "{s:[]}", "foo");
    if(!json_is_object(value) || json_object_size(value) != 1)
        fail("json_pack array failed");
    if(!json_is_array(json_object_get(value, "foo")))
        fail("json_pack array failed");
    if(json_object_get(value, "foo")->refcount != (ssize_t)1)
        fail("json_pack object refcount failed");
    json_decref(value);

    /* simple array */
    value = json_pack_ex(&error, 0, "[i,i,i]", 0, 1, 2);
    if(!json_is_array(value) || json_array_size(value) != 3)
        fail("json_pack object failed");
    for(i=0; i<3; i++)
    {
        if(!json_is_integer(json_array_get(value, i)) ||
           json_integer_value(json_array_get(value, i)) != i)

            fail("json_pack integer array failed");
    }
    json_decref(value);

    /* Whitespace; regular string */
    value = json_pack_ex(&error, 0, " s ", "test");
    if(!json_is_string(value) || strcmp("test", json_string_value(value)))
        fail("json_pack string (with whitespace) failed");
    json_decref(value);

    /* Whitespace; empty array */
    value = json_pack_ex(&error, 0, "[ ]");
    if(!json_is_array(value) || json_array_size(value) != 0)
        fail("json_pack empty array (with whitespace) failed");
    json_decref(value);

    /* Whitespace; array */
    value = json_pack_ex(&error, 0, "[ i , i,  i ] ", 1, 2, 3);
    if(!json_is_array(value) || json_array_size(value) != 3)
        fail("json_pack array (with whitespace) failed");
    json_decref(value);

    /*
     * Invalid cases
     */

    /* mismatched open/close array/object */
    if(json_pack_ex(&error, 0, "[}"))
        fail("json_pack failed to catch mismatched '}'");
    if(error.line != 1 || error.column != 2)
        fail("json_pack didn't get the error coordinates right!");

    if(json_pack_ex(&error, 0, "{]"))
        fail("json_pack failed to catch mismatched ']'");
    if(error.line != 1 || error.column != 2)
        fail("json_pack didn't get the error coordinates right!");

    /* missing close array */
    if(json_pack_ex(&error, 0, "["))
        fail("json_pack failed to catch missing ']'");
    if(error.line != 1 || error.column != 2)
        fail("json_pack didn't get the error coordinates right!");

    /* missing close object */
    if(json_pack_ex(&error, 0, "{"))
        fail("json_pack failed to catch missing '}'");
    if(error.line != 1 || error.column != 2)
        fail("json_pack didn't get the error coordinates right!");

    /* NULL string */
    if(json_pack_ex(&error, 0, "s", NULL))
        fail("json_pack failed to catch null argument string");
    if(error.line != 1 || error.column != 1)
        fail("json_pack didn't get the error coordinates right!");

    /* NULL format */
    if(json_pack_ex(&error, 0, NULL))
        fail("json_pack failed to catch NULL format string");
    if(error.line != -1 || error.column != -1)
        fail("json_pack didn't get the error coordinates right!");

    /* More complicated checks for row/columns */
    if(json_pack_ex(&error, 0, "{ {}: s }", "foo"))
        fail("json_pack failed to catch object as key");
    if(error.line != 1 || error.column != 3)
        fail("json_pack didn't get the error coordinates right!");

    if(json_pack_ex(&error, 0, "{ s: {},  s:[ii{} }", "foo", "bar", 12, 13))
        fail("json_pack failed to catch missing ]");
    if(error.line != 1 || error.column != 19)
        fail("json_pack didn't get the error coordinates right!");

    if(json_pack_ex(&error, 0, "[[[[[   [[[[[  [[[[ }]]]] ]]]] ]]]]]"))
        fail("json_pack failed to catch extra }");
    if(error.line != 1 || error.column != 21)
        fail("json_pack didn't get the error coordinates right!");

    return 0;
}
