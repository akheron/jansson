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
    json_t *array, *five, *seven, *value;
    int i;

    array = json_array();
    five = json_integer(5);
    seven = json_integer(7);

    if(!array)
        fail("unable to create array");
    if(!five)
        fail("unable to create integer");
    if(!seven)
        fail("unable to create integer");

    if(json_array_size(array) != 0)
        fail("empty array has nonzero size");

    if(json_array_append(array, five))
        fail("unable to append");

    if(json_array_size(array) != 1)
        fail("wrong array size");

    value = json_array_get(array, 0);
    if(!value)
        fail("unable to get item");
    if(value != five)
        fail("got wrong value");

    if(json_array_append(array, seven))
        fail("unable to append value");

    if(json_array_size(array) != 2)
        fail("wrong array size");

    value = json_array_get(array, 1);
    if(!value)
        fail("unable to get item");
    if(value != seven)
        fail("got wrong value");

    if(json_array_set(array, 0, seven))
        fail("unable to set value");

    if(json_array_size(array) != 2)
        fail("wrong array size");

    value = json_array_get(array, 0);
    if(!value)
        fail("unable to get item");
    if(value != seven)
        fail("got wrong value");

    if(json_array_get(array, 2) != NULL)
        fail("able to get value out of bounds");

    if(!json_array_set(array, 2, seven))
        fail("able to set value out of bounds");

    for(i = 2; i < 30; i++) {
        if(json_array_append(array, seven))
            fail("unable to append value");

        if(json_array_size(array) != i + 1)
            fail("wrong array size");
    }

    for(i = 0; i < 30; i++) {
        value = json_array_get(array, i);
        if(!value)
            fail("unable to get item");
        if(value != seven)
            fail("got wrong value");
    }

    json_decref(five);
    json_decref(seven);
    json_decref(array);

    return 0;
}
