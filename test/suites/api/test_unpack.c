/*
 * Copyright (c) 2009, 2010 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
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

    /*
     * Simple, valid json_pack cases
     */

    /* true */
    rv = json_unpack(json_true(), NULL, "b", &i1);
    if(rv || !i1)
        fail("json_unpack boolean failed");

    /* false */
    rv = json_unpack(json_false(), NULL, "b", &i1);
    if(rv || i1)
        fail("json_unpack boolean failed");

    /* null */
    rv = json_unpack(json_null(), NULL, "n");
    if(rv)
        fail("json_unpack null failed");

    /* integer */
    j = json_integer(1);
    rv = json_unpack(j, NULL, "i", &i1);
    if(rv || i1 != 1)
        fail("json_unpack integer failed");
    json_decref(j);

    /* real */
    j = json_real(1.0);
    rv = json_unpack(j, NULL, "f", &f);
    if(rv || f != 1.0)
        fail("json_unpack real failed");
    json_decref(j);

    /* string */
    j = json_string("foo");
    rv = json_unpack(j, NULL, "s", &s);
    if(rv || strcmp(s, "foo"))
        fail("json_unpack string failed");
    json_decref(j);

    /* empty object */
    j = json_object();
    rv = json_unpack(j, NULL, "{}");
    if(rv)
        fail("json_unpack empty object failed");
    json_decref(j);

    /* empty list */
    j = json_array();
    rv = json_unpack(j, NULL, "[]");
    if(rv)
        fail("json_unpack empty list failed");
    json_decref(j);

    /* non-incref'd object */
    j = json_object();
    rv = json_unpack(j, NULL, "o", &j2);
    if(j2 != j || j->refcount != (ssize_t)1)
        fail("json_unpack object failed");
    json_decref(j);

    /* incref'd object */
    j = json_object();
    rv = json_unpack(j, NULL, "O", &j2);
    if(j2 != j || j->refcount != (ssize_t)2)
        fail("json_unpack object failed");
    json_decref(j);
    json_decref(j);

    /* simple object */
    j = json_pack(NULL, "{s:i}", "foo", 1);
    rv = json_unpack(j, NULL, "{s:i}", "foo", &i1);
    if(rv || i1!=1)
        fail("json_unpack simple object failed");
    json_decref(j);

    /* simple array */
    j = json_pack(NULL, "[iii]", 1, 2, 3);
    rv = json_unpack(j, NULL, "[i,i,i]", &i1, &i2, &i3);
    if(rv || i1 != 1 || i2 != 2 || i3 != 3)
        fail("json_unpack simple array failed");
    json_decref(j);

    return 0;
}

/* vim: ts=4:expandtab:sw=4
 */
