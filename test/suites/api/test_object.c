/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <string.h>
#include "util.h"

static void test_clear()
{
    json_t *object, *ten;

    object = json_object();
    ten = json_integer(10);

    if(!object)
        fail("unable to create object");
    if(!ten)
        fail("unable to create integer");

    if(json_object_set(object, "a", ten) ||
       json_object_set(object, "b", ten) ||
       json_object_set(object, "c", ten) ||
       json_object_set(object, "d", ten) ||
       json_object_set(object, "e", ten))
        fail("unable to set value");

    if(json_object_size(object) != 5)
        fail("invalid size");

    json_object_clear(object);

    if(json_object_size(object) != 0)
        fail("invalid size after clear");

    json_decref(ten);
    json_decref(object);
}

static void test_update()
{
    json_t *object, *other, *nine, *ten;

    object = json_object();
    other = json_object();

    nine = json_integer(9);
    ten = json_integer(10);

    if(!object || !other)
        fail("unable to create object");
    if(!nine || !ten)
        fail("unable to create integer");


    /* update an empty object with an empty object */

    if(json_object_update(object, other))
        fail("unable to update an emtpy object with an empty object");

    if(json_object_size(object) != 0)
        fail("invalid size after update");

    if(json_object_size(other) != 0)
        fail("invalid size for updater after update");


    /* update an empty object with a nonempty object */

    if(json_object_set(other, "a", ten) ||
       json_object_set(other, "b", ten) ||
       json_object_set(other, "c", ten) ||
       json_object_set(other, "d", ten) ||
       json_object_set(other, "e", ten))
        fail("unable to set value");

    if(json_object_update(object, other))
        fail("unable to update an empty object");

    if(json_object_size(object) != 5)
        fail("invalid size after update");

    if(json_object_get(object, "a") != ten ||
       json_object_get(object, "b") != ten ||
       json_object_get(object, "c") != ten ||
       json_object_get(object, "d") != ten ||
       json_object_get(object, "e") != ten)
        fail("update works incorrectly");


    /* perform the same update again */

    if(json_object_update(object, other))
        fail("unable to update an empty object");

    if(json_object_size(object) != 5)
        fail("invalid size after update");

    if(json_object_get(object, "a") != ten ||
       json_object_get(object, "b") != ten ||
       json_object_get(object, "c") != ten ||
       json_object_get(object, "d") != ten ||
       json_object_get(object, "e") != ten)
        fail("update works incorrectly");


    /* update a nonempty object with a nonempty object with both old
       and new keys */

    if(json_object_clear(other))
        fail("clear failed");

    if(json_object_set(other, "a", nine) ||
       json_object_set(other, "b", nine) ||
       json_object_set(other, "f", nine) ||
       json_object_set(other, "g", nine) ||
       json_object_set(other, "h", nine))
        fail("unable to set value");

    if(json_object_update(object, other))
        fail("unable to update a nonempty object");

    if(json_object_size(object) != 8)
        fail("invalid size after update");

    if(json_object_get(object, "a") != nine ||
       json_object_get(object, "b") != nine ||
       json_object_get(object, "f") != nine ||
       json_object_get(object, "g") != nine ||
       json_object_get(object, "h") != nine)
        fail("update works incorrectly");

    json_decref(nine);
    json_decref(ten);
    json_decref(other);
    json_decref(object);
}

static void test_circular()
{
    json_t *object1, *object2;

    object1 = json_object();
    object2 = json_object();
    if(!object1 || !object2)
        fail("unable to create object");

    /* the simple case is checked */
    if(json_object_set(object1, "a", object1) == 0)
        fail("able to set self");

    /* create circular references */
    if(json_object_set(object1, "a", object2) ||
       json_object_set(object2, "a", object1))
        fail("unable to set value");

    /* circularity is detected when dumping */
    if(json_dumps(object1, 0) != NULL)
        fail("able to dump circulars");

    /* decref twice to deal with the circular references */
    json_decref(object1);
    json_decref(object2);
    json_decref(object1);
}

static void test_misc()
{
    json_t *object, *string, *other_string, *value;
    void *iter;

    object = json_object();
    string = json_string("test");
    other_string = json_string("other");

    if(!object)
        fail("unable to create object");
    if(!string || !other_string)
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
}

int main()
{
    test_misc();
    test_clear();
    test_update();
    test_circular();

    return 0;
}
