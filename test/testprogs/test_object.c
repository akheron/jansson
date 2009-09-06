/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include "util.h"

int main()
{
    json_t *object, *string, *other_string, *value;

    object = json_object();
    string = json_string("test");
    other_string = json_string("other");

    if(!object)
        fail("unable to create object");
    if(!string)
        fail("unable to create string");
    if(!other_string)
        fail("unable to create string");

    if(json_object_get(object, "a"))
        fail("value for nonexisting key");

    if(json_object_set(object, "a", string))
        fail("unable to set value");

    if(!json_object_set(object, NULL, string))
        fail("able to set NULL key");

    if(!json_object_set(object, "a", NULL))
        fail("able to set NULL value");

    iter = json_object_iter(object);
    if(!iter)
        fail("unable to get iterator");

    if(strcmp(json_object_iter_key(iter), "a"))
        fail("iterating failed: wrong key");
    if(json_object_iter_value(iter) != string)
        fail("iterating failed: wrong value");
    if(json_object_iter_next(object, iter) != NULL)
        fail("able to iterate over the end");

    /* invalid UTF-8 in key */
    if(!json_object_set(object, "a\xefz", string))
        fail("able to set invalid unicode key");

    value = json_object_get(object, "a");
    if(!value)
        fail("no value for existing key");
    if(value != string)
        fail("got different value than what was added");

    /* "a", "lp" and "px" collide in a five-bucket hashtable */
    if(json_object_set(object, "b", string) ||
       json_object_set(object, "lp", string) ||
       json_object_set(object, "px", string))
        fail("unable to set value");

    value = json_object_get(object, "a");
    if(!value)
        fail("no value for existing key");
    if(value != string)
        fail("got different value than what was added");

    if(json_object_set(object, "a", other_string))
        fail("unable to replace an existing key");

    value = json_object_get(object, "a");
    if(!value)
        fail("no value for existing key");
    if(value != other_string)
        fail("got different value than what was set");

    if(!json_object_del(object, "nonexisting"))
        fail("able to delete a nonexisting key");

    if(json_object_del(object, "px"))
        fail("unable to delete an existing key");

    if(json_object_del(object, "a"))
        fail("unable to delete an existing key");

    if(json_object_del(object, "lp"))
        fail("unable to delete an existing key");


    /* add many keys to initiate rehashing */

    if(json_object_set(object, "a", string))
        fail("unable to set value");

    if(json_object_set(object, "lp", string))
        fail("unable to set value");

    if(json_object_set(object, "px", string))
        fail("unable to set value");

    if(json_object_set(object, "c", string))
        fail("unable to set value");

    if(json_object_set(object, "d", string))
        fail("unable to set value");

    if(json_object_set(object, "e", string))
        fail("unable to set value");


    if(json_object_set_new(object, "foo", json_integer(123)))
        fail("unable to set new value");

    value = json_object_get(object, "foo");
    if(!json_is_integer(value) || json_integer_value(value) != 123)
      fail("json_object_set_new works incorrectly");

    if(!json_object_set_new(object, NULL, json_integer(432)))
        fail("able to set_new NULL key");

    if(!json_object_set_new(object, "foo", NULL))
        fail("able to set_new NULL value");

    json_decref(string);
    json_decref(other_string);
    json_decref(object);

    return 0;
}
