/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

#define BUFFER_SIZE (256 * 1024)

int main(int argc, char *argv[])
{
    json_t *json;
    json_error_t error;
    int count;
    char buffer[BUFFER_SIZE];
    char *result;

    if(argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }

    count = fread(buffer, 1, BUFFER_SIZE, stdin);
    if(count < 0 || count >= BUFFER_SIZE) {
      fprintf(stderr, "unable to read input\n");
      return 1;
    }
    buffer[count] = '\0';

    json = json_loads(buffer, &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    result = json_dumps(json, 0);
    json_decref(json);

    puts(result);
    free(result);

    return 0;
}
