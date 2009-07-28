/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <jansson.h>

int main(int argc, char *argv[])
{
    json_t *json;
    json_error_t error;

    if(argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }

    json = json_loadf(stdin, &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    /* loadf_dumpf indents, others don't, so dumping with and without
       indenting is tested */
    json_dumpf(json, stdout, JSON_INDENT(4));
    json_decref(json);

    return 0;
}
