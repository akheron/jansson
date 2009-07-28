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

    if(argc != 3) {
        fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
        return 2;
    }

    json = json_load_file(argv[1], &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    json_dump_file(json, argv[2], 0);
    json_decref(json);

    return 0;
}
