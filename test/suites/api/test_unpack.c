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
    json_t *j, *j2;
    int i1, i2, i3;
    int rv;
    //void* v;
    double f;
    char *s;

    json_error_t error;

    /*
     * Simple, valid json_pack cases
     */

    /* true */
    rv = json_unpack(json_true(), &error, "b", &i1);
    if(rv || !i1)
        fail("json_unpack boolean failed");

    /* false */
    rv = json_unpack(json_false(), &error, "b", &i1);
    if(rv || i1)
        fail("json_unpack boolean failed");

    /* null */
    rv = json_unpack(json_null(), &error, "n");
    if(rv)
        fail("json_unpack null failed");

    /* integer */
    j = json_integer(1);
    rv = json_unpack(j, &error, "i", &i1);
    if(rv || i1 != 1)
        fail("json_unpack integer failed");
    json_decref(j);

    /* real */
    j = json_real(1.0);
    rv = json_unpack(j, &error, "f", &f);
    if(rv || f != 1.0)
        fail("json_unpack real failed");
    json_decref(j);

    /* string */
    j = json_string("foo");
    rv = json_unpack(j, &error, "s", &s);
    if(rv || strcmp(s, "foo"))
        fail("json_unpack string failed");
    json_decref(j);

    /* empty object */
    j = json_object();
    rv = json_unpack(j, &error, "{}");
    if(rv)
        fail("json_unpack empty object failed");
    json_decref(j);

    /* empty list */
    j = json_array();
    rv = json_unpack(j, &error, "[]");
    if(rv)
        fail("json_unpack empty list failed");
    json_decref(j);

    /* non-incref'd object */
    j = json_object();
    rv = json_unpack(j, &error, "o", &j2);
    if(j2 != j || j->refcount != (ssize_t)1)
        fail("json_unpack object failed");
    json_decref(j);

    /* incref'd object */
    j = json_object();
    rv = json_unpack(j, &error, "O", &j2);
    if(j2 != j || j->refcount != (ssize_t)2)
        fail("json_unpack object failed");
    json_decref(j);
    json_decref(j);

    /* simple object */
    j = json_pack(&error, "{s:i}", "foo", 1);
    rv = json_unpack(j, &error, "{s:i}", "foo", &i1);
    if(rv || i1!=1)
        fail("json_unpack simple object failed");
    json_decref(j);

    /* simple array */
    j = json_pack(&error, "[iii]", 1, 2, 3);
    rv = json_unpack(j, &error, "[i,i,i]", &i1, &i2, &i3);
    if(rv || i1 != 1 || i2 != 2 || i3 != 3)
        fail("json_unpack simple array failed");
    json_decref(j);

    /*
     * Invalid cases
     */

    /* mismatched open/close array/object */
    j = json_pack(&error, "[]");
    rv = json_unpack(j, &error, "[}");
    if(!rv)
        fail("json_unpack failed to catch mismatched ']'");
    json_decref(j);

    j = json_pack(&error, "{}");
    rv = json_unpack(j, &error, "{]");
    if(!rv)
        fail("json_unpack failed to catch mismatched '}'");
    json_decref(j);

    /* missing close array */
    j = json_pack(&error, "[]");
    rv = json_unpack(j, &error, "[");
    if(rv >= 0)
        fail("json_unpack failed to catch missing ']'");
    json_decref(j);

    /* missing close object */
    j = json_pack(&error, "{}");
    rv = json_unpack(j, &error, "{");
    if(rv >= 0)
        fail("json_unpack failed to catch missing '}'");
    json_decref(j);

    /* NULL format string */
    j = json_pack(&error, "[]");
    rv =json_unpack(j, &error, NULL);
    if(rv >= 0)
        fail("json_unpack failed to catch null format string");
    json_decref(j);

    /* NULL string pointer */
    j = json_string("foobie");
    rv =json_unpack(j, &error, "s", NULL);
    if(rv >= 0)
        fail("json_unpack failed to catch null string pointer");
    json_decref(j);

    return 0;

    //fprintf(stderr, "%i/%i: %s %s\n", error.line, error.column, error.source, error.text);
}

/* vim: ts=4:expandtab:sw=4
 */
