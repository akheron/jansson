/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "util.h"
#include <jansson.h>

static void test_compare_simple() {
    json_t *value1, *value2;

    if (json_compare(NULL, NULL) != 0)
        fail("json_compare returns wrong result for two NULLs");

    value1 = json_true();
    if (json_compare(value1, NULL) != 1 || json_compare(NULL, value1) != -1)
        fail("json_compare returns wrong result for true/NULL");

    /* this covers true, false and null as they are singletons */
    if (json_compare(value1, value1) != 0)
        fail("identical objects are not equal");
    json_decref(value1);

    /* integer */
    value1 = json_integer(1);
    value2 = json_integer(1);
    if (!value1 || !value2)
        fail("unable to create integers");
    if (json_compare(value1, value2) != 0)
        fail("json_compare returns wrong result for two equal integers");
    json_decref(value2);

    value2 = json_integer(2);
    if (!value2)
        fail("unable to create an integer");
    if (json_compare(value1, value2) != -1)
        fail("json_compare returns wrong result for two inequal integers");
    if (json_compare(value2, value1) != 1)
        fail("json_compare returns wrong result for two inequal integers");

    json_decref(value1);
    json_decref(value2);

    /* real */
    value1 = json_real(1.2);
    value2 = json_real(1.2);
    if (!value1 || !value2)
        fail("unable to create reals");
    if (json_compare(value1, value2) != 0)
        fail("json_compare returns wrong result for two equal reals");
    json_decref(value2);

    value2 = json_real(3.141592);
    if (!value2)
        fail("unable to create an real");
    if (json_compare(value1, value2) != -1)
        fail("json_compare returns wrong result for two inequal reals");
    if (json_compare(value2, value1) != 1)
        fail("json_compare returns wrong result for two inequal reals");

    json_decref(value1);
    json_decref(value2);

    /* string */
    value1 = json_string("foo");
    value2 = json_string("foo");
    if (!value1 || !value2)
        fail("unable to create strings");
    if (json_compare(value1, value2) != 0)
        fail("json_compare returns wrong result for two equal strings");
    json_decref(value2);

    value2 = json_string("bar");
    if (!value2)
        fail("unable to create an string");
    if (json_compare(value1, value2) != 1)
        fail("json_compare returns wrong result for two inequal strings");
    if (json_compare(value2, value1) != -1)
        fail("json_compare returns wrong result for two inequal strings");
    json_decref(value2);

    value2 = json_string("foo2");
    if (!value2)
        fail("unable to create an string");
    if (json_compare(value1, value2) != -1)
        fail("json_compare returns wrong result for for two inequal length strings");
    if (json_compare(value2, value1) != 1)
        fail("json_compare returns wrong result for for two inequal length strings");

    json_decref(value1);
    json_decref(value2);
}

static void test_compare_array() {
    json_t *array1, *array2;

    array1 = json_array();
    array2 = json_array();
    if (!array1 || !array2)
        fail("unable to create arrays");

    if (json_compare(array1, array2) != 0)
        fail("json_compare returns wrong result for two empty arrays");

    json_array_append_new(array1, json_integer(1));
    json_array_append_new(array2, json_integer(1));
    json_array_append_new(array1, json_string("foo"));
    json_array_append_new(array2, json_string("foo"));
    json_array_append_new(array1, json_integer(2));
    json_array_append_new(array2, json_integer(2));
    if (json_compare(array1, array2) != 0)
        fail("json_compare returns wrong result for two equal arrays");

    json_array_remove(array2, 2);
    if (json_compare(array1, array2) != -1)
        fail("json_compare returns wrong result for two inequal arrays");
    if (json_compare(array2, array1) != 1)
        fail("json_compare returns wrong result for two inequal arrays");

    json_array_append_new(array2, json_integer(0));
    if (json_compare(array1, array2) != 1)
        fail("json_compare returns wrong result for two inequal arrays");
    if (json_compare(array2, array1) != -1)
        fail("json_compare returns wrong result for two inequal arrays");

    json_decref(array1);
    json_decref(array2);
}

static void test_compare_object() {
    json_t *object1, *object2;

    object1 = json_object();
    object2 = json_object();
    if (!object1 || !object2)
        fail("unable to create objects");

    if (json_compare(object1, object2) != 0)
        fail("json_compare returns wrong result for two empty objects");

    json_object_set_new(object1, "a", json_integer(1));
    json_object_set_new(object1, "b", json_string("foo"));
    json_object_set_new(object1, "c", json_integer(2));

    /* Populate object2 in reverse to ensure comparison is not affected by
     * insertion order.
    */
    json_object_set_new(object2, "c", json_integer(2));
    json_object_set_new(object2, "b", json_string("foo"));
    json_object_set_new(object2, "a", json_integer(1));

    if (json_compare(object1, object2) != 0)
        fail("json_compare returns the wrong result for two equal objects");

    json_object_del(object2, "c");
    if (json_compare(object1, object2) != 1)
        fail("json_compare returns the wrong result for two inequal objects (missing key)");
    if (json_compare(object2, object1) != -1)
        fail("json_compare returns the wrong result for two inequal objects (missing key)");

    json_object_set_new(object2, "d", json_integer(2));
    if (json_compare(object1, object2) != -1)
        fail("json_compare returns the wrong result for two inequal objects (different key)");
    if (json_compare(object2, object1) != 1)
        fail("json_compare returns the wrong result for two inequal objects (different key)");

    json_object_del(object2, "c");
    json_object_set_new(object2, "c", json_integer(1));
    if (json_compare(object1, object2) != -1)
        fail("json_compare returns the wrong result for two inequal objects (different value)");
    if (json_compare(object2, object1) != 1)
        fail("json_compare returns the wrong result for two inequal objects (different value)");

    json_decref(object1);
    json_decref(object2);
}

static void run_tests() {
    test_compare_simple();
    test_compare_array();
    test_compare_object();
}
