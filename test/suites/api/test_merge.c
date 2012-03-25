/*
 * Copyright (c) 2012 Rogerz Zhang <rogerz.zhang@gmail.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <string.h>
#include "util.h"

static void compare(json_t *result, json_t *expect)
{
    char *r = json_dumps(result, JSON_INDENT(2));
    char *e = json_dumps(expect, JSON_INDENT(2));
    fprintf(stderr, "result:\n%s\nexpect:\n%s\n", r, e);
    free(r);
    free(e);
}
static void test_object(void)
{
    json_t *origin, *ours, *theirs, *conflict, *merged;

    origin = json_pack("{sssisssi}", "os", "old", "oi", 101, "bs", "both old", "bi", 301);
    theirs = json_pack("{sssisssi}", "ns", "new", "ni", 202, "bs", "both new", "bi", 302);
    conflict = json_pack("{sisssiss}", "ns", 202, "ni", "new", "bs", 302, "bi", "both new");

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, 0);
    merged = json_pack("{sssisssisssi}", "os", "old", "oi", 101, "bs", "both new", "bi", 302, "ns", "new", "ni", 202);
    if (!json_equal(ours, merged)) {
        compare(ours, merged);
        fail("json_merge() fails to merge object");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &conflict, 0);
    merged = json_pack("{sssisssisssi}", "os", "old", "oi", 101, "bs", "both old", "bi", 301, "ni", "new", "ns", 202);
    if (!json_equal(ours, merged)) {
        compare(ours, merged);
        fail("json_merge() fails to reject conflict");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_MISSING);
    merged = json_pack("{sssisssi}", "os", "old", "oi", 101, "bs", "both new", "bi", 302);
    if (!json_equal(ours, merged)) {
        compare(ours, merged);
        fail("json_merge() fails to skip missing");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_EXISTING);
    merged = json_pack("{sssisssisssi}", "os", "old", "oi", 101, "bs", "both old", "bi", 301, "ns", "new", "ni", 202);
    if (!json_equal(ours, merged)) {
        compare(ours, merged);
        fail("json_merge() fails to overwrite existing");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_EXISTING | JSON_SKIP_MISSING);
    if (!json_equal(ours, origin)) {
        compare(ours, merged);
        fail("json_merge() fails to skip all");
    }
    json_decref(ours);
    json_decref(merged);

    json_decref(origin);
    json_decref(theirs);
}

static void test_array(void)
{
    json_t *origin, *ours, *theirs, *merged;

    origin = json_loads("[11, 12]", 0, NULL);
    theirs = json_loads("[21, 22, 23, 24]", 0, NULL);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, 0);
    merged = json_loads("[21, 22, 23, 24]", 0, NULL);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to merge array");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_MISSING);
    merged = json_loads("[11, 12]", 0, NULL);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to skip extending array");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_EXISTING);
    merged = json_loads("[11, 12, 21, 22, 23, 24]", 0, NULL);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to overwrite array");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_MISSING | JSON_SKIP_EXISTING);
    merged = json_loads("[11, 12]", 0, NULL);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to skip all");
    }
    json_decref(ours);
    json_decref(merged);

    json_decref(origin);
    json_decref(theirs);
}


static void test_recursive()
{
    json_t *ours, *merged;
    json_t *origin = json_pack("{s:{si},s:{sisi}}", "o", "o.o", 10101, "b", "b.o", 30101, "b.b", 30301);
    json_t *theirs = json_pack("{s:{si},s:{sisi}}", "n", "n.n", 20202, "b", "b.n", 30202, "b.b", 30302);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, 0);
    merged = json_pack("{s:{si},s:{sisisi},s:{si}}"
        , "o", "o.o", 10101, "b", "b.o", 30101, "b.n", 30202, "b.b", 30302, "n", "n.n", 20202);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to merge recursively");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_MISSING);
    merged = json_pack("{s:{si},s:{sisi}}"
        , "o", "o.o", 10101, "b", "b.o", 30101, "b.b", 30302);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to skip missing recursively");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_EXISTING);
    merged = json_pack("{s:{si},s:{sisisi},s:{si}}"
        , "o", "o.o", 10101, "b", "b.o", 30101, "b.n", 30202, "b.b", 30301, "n", "n.n", 20202);
    if (!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to skip existing recursively");
    }
    json_decref(ours);
    json_decref(merged);

    ours = json_deep_copy(origin);
    json_merge(&ours, &theirs, JSON_SKIP_MISSING | JSON_SKIP_EXISTING);
    if (!json_equal(ours, origin)){
        compare(ours, origin);
        fail("json_merge() fails to skip all recursively");
    }
    json_decref(ours);
    json_decref(merged);

    json_decref(origin);
    json_decref(theirs);
}

static void test_deep_in()
{
    json_t *ours, *theirs, *merged;
    json_t *origin = json_pack("{s:[{si}, {si}]}", "b", "b.b", 30101, "b.b", 30201);

    ours = json_deep_copy(origin);
    theirs = json_pack("{s:[{si}]}", "b", "b.b", 30102);
    json_merge(&ours, &theirs, JSON_DEEP_IN_ARRAY);
    if(!json_equal(ours, origin)){
        compare(ours, origin);
        fail("json_merge() fails to reject shorter array");
    }
    json_decref(ours);
    json_decref(theirs);

    ours = json_deep_copy(origin);
    theirs = json_pack("{s:[{si},{si}]}", "b", "b.b", 30102, "b.b", 30202);
    json_merge(&ours, &theirs, JSON_DEEP_IN_ARRAY);
    if(!json_equal(ours, theirs)){
        compare(ours, theirs);
        fail("json_merge() fails to deep in array");
    }
    json_decref(ours);
    json_decref(theirs);

    ours = json_deep_copy(origin);
    theirs = json_pack("{s:[{si},i]}", "b", "b.b", 30102, 30202);
    merged = json_pack("{s:[{si},{si}]}", "b", "b.b", 30102, "b.b", 30201);
    json_merge(&ours, &theirs, JSON_DEEP_IN_ARRAY);
    if(!json_equal(ours, merged)){
        compare(ours, merged);
        fail("json_merge() fails to reject conflict when deep in array");
    }
    json_decref(ours);
    json_decref(theirs);

    ours = json_deep_copy(origin);
    theirs = json_pack("{s:[{si},{si},{si}]}", "b", "b.b", 30102, "b.b", 30202, "b.b", 30302);
    json_merge(&ours, &theirs, JSON_DEEP_IN_ARRAY);
    if(!json_equal(ours, origin)){
        compare(ours, origin);
        fail("json_merge() fails to reject longer array");
    }
    json_decref(ours);
    json_decref(theirs);
}

static void run_tests()
{
    test_object();
    test_array();
    test_recursive();
    test_deep_in();
}
