/*
 * Copyright (c) 2009, 2010 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

static int getenv_int(const char *name)
{
    char *value, *end;
    long result;

    value = getenv(name);
    if(!value)
        return 0;

    result = strtol(value, &end, 10);
    if(*end != '\0')
        return 0;

    return (int)result;
}

int main(int argc, char *argv[])
{
    int indent = 0;
    unsigned int flags = 0;

    json_t *json;
    json_error_t error;

    if(argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }

    indent = getenv_int("JSON_INDENT");
    if(indent < 0 || indent > 255) {
        fprintf(stderr, "invalid value for JSON_INDENT: %d\n", indent);
        return 2;
    }

    if(indent > 0)
        flags |= JSON_INDENT(indent);

    if(getenv_int("JSON_COMPACT") > 0)
        flags |= JSON_COMPACT;

    if(getenv_int("JSON_ENSURE_ASCII"))
        flags |= JSON_ENSURE_ASCII;

    if(getenv_int("JSON_PRESERVE_ORDER"))
        flags |= JSON_PRESERVE_ORDER;

    if(getenv_int("JSON_SORT_KEYS"))
        flags |= JSON_SORT_KEYS;

    json = json_loadf(stdin, &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    json_dumpf(json, stdout, flags);
    json_decref(json);

    return 0;
}
