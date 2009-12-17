/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include "jansson_private.h"
#include "strbuffer.h"

#define MAX_INTEGER_STR_LENGTH  100
#define MAX_REAL_STR_LENGTH     100

typedef int (*dump_func)(const char *buffer, int size, void *data);

struct string
{
    char *buffer;
    int length;
    int size;
};

static int dump_to_strbuffer(const char *buffer, int size, void *data)
{
    return strbuffer_append_bytes((strbuffer_t *)data, buffer, size);
}

static int dump_to_file(const char *buffer, int size, void *data)
{
    FILE *dest = (FILE *)data;
    if(fwrite(buffer, size, 1, dest) != 1)
        return -1;
    return 0;
}

/* 256 spaces (the maximum indentation size) */
static char whitespace[] = "                                                                                                                                                                                                                                                                ";

static int dump_indent(unsigned long flags, int depth, dump_func dump, void *data)
{
    if(JSON_INDENT(flags) > 0)
    {
        int i, ws_count = JSON_INDENT(flags);

        if(dump("\n", 1, data))
            return -1;

        for(i = 0; i < depth; i++)
        {
            if(dump(whitespace, ws_count, data))
                return -1;
        }
    }
    return 0;
}

static int dump_string(const char *str, dump_func dump, void *data)
{
    const char *end;

    if(dump("\"", 1, data))
        return -1;

    end = str;
    while(1)
    {
        const char *text;
        char seq[7];
        int length;

        while(*end && *end != '\\' && *end != '"' && (unsigned char)*end > 0x1F)
            end++;

        if(end != str) {
            if(dump(str, end - str, data))
                return -1;
        }

        if(!*end)
            break;

        /* handle \, ", and control codes */
        length = 2;
        switch(*end)
        {
            case '\\': text = "\\\\"; break;
            case '\"': text = "\\\""; break;
            case '\b': text = "\\b"; break;
            case '\f': text = "\\f"; break;
            case '\n': text = "\\n"; break;
            case '\r': text = "\\r"; break;
            case '\t': text = "\\t"; break;
            default:
            {
                sprintf(seq, "\\u00%02x", *end);
                text = seq;
                length = 6;
                break;
            }
        }

        if(dump(text, length, data))
            return -1;

        end++;
        str = end;
    }

    return dump("\"", 1, data);
}

static int do_dump(const json_t *json, unsigned long flags, int depth,
                   dump_func dump, void *data)
{
    switch(json_typeof(json)) {
        case JSON_NULL:
            return dump("null", 4, data);

        case JSON_TRUE:
            return dump("true", 4, data);

        case JSON_FALSE:
            return dump("false", 5, data);

        case JSON_INTEGER:
        {
            char buffer[MAX_INTEGER_STR_LENGTH];
            int size;

            size = snprintf(buffer, MAX_INTEGER_STR_LENGTH, "%d", json_integer_value(json));
            if(size >= MAX_INTEGER_STR_LENGTH)
                return -1;

            return dump(buffer, size, data);
        }

        case JSON_REAL:
        {
            char buffer[MAX_REAL_STR_LENGTH];
            int size;

            size = snprintf(buffer, MAX_REAL_STR_LENGTH, "%.17g",
                            json_real_value(json));
            if(size >= MAX_REAL_STR_LENGTH)
                return -1;

            /* Make sure there's a dot or 'e' in the output. Otherwise
               a real is converted to an integer when decoding */
            if(strchr(buffer, '.') == NULL &&
               strchr(buffer, 'e') == NULL)
            {
                if(size + 2 >= MAX_REAL_STR_LENGTH) {
                    /* No space to append ".0" */
                    return -1;
                }
                buffer[size] = '.';
                buffer[size + 1] = '0';
                size += 2;
            }

            return dump(buffer, size, data);
        }

        case JSON_STRING:
            return dump_string(json_string_value(json), dump, data);

        case JSON_ARRAY:
        {
            int i;
            int n;
            json_array_t *array;

            /* detect circular references */
            array = json_to_array(json);
            if(array->visited)
                return -1;
            array->visited = 1;

            n = json_array_size(json);

            if(dump("[", 1, data))
                return -1;
            if(n == 0)
                return dump("]", 1, data);
            if(dump_indent(flags, depth + 1, dump, data))
                return -1;

            for(i = 0; i < n; ++i) {
                if(do_dump(json_array_get(json, i), flags, depth + 1,
                           dump, data))
                    return -1;

                if(i < n - 1)
                {
                    if(dump(",", 1, data) ||
                       dump_indent(flags, depth + 1, dump, data))
                        return -1;
                }
                else
                {
                    if(dump_indent(flags, depth, dump, data))
                        return -1;
                }
            }

            array->visited = 0;
            return dump("]", 1, data);
        }

        case JSON_OBJECT:
        {
            json_object_t *object;
            void *iter;

            /* detect circular references */
            object = json_to_object(json);
            if(object->visited)
                return -1;
            object->visited = 1;

            iter = json_object_iter((json_t *)json);

            if(dump("{", 1, data))
                return -1;
            if(!iter)
                return dump("}", 1, data);
            if(dump_indent(flags, depth + 1, dump, data))
                return -1;

            while(iter)
            {
                void *next = json_object_iter_next((json_t *)json, iter);

                dump_string(json_object_iter_key(iter), dump, data);
                if(dump(": ", 2, data) ||
                   do_dump(json_object_iter_value(iter), flags, depth + 1,
                           dump, data))
                    return -1;

                if(next)
                {
                    if(dump(",", 1, data) ||
                       dump_indent(flags, depth + 1, dump, data))
                        return -1;
                }
                else
                {
                    if(dump_indent(flags, depth, dump, data))
                        return -1;
                }

                iter = next;
            }

            object->visited = 0;
            return dump("}", 1, data);
        }

        default:
            /* not reached */
            return -1;
    }
}


char *json_dumps(const json_t *json, unsigned long flags)
{
    strbuffer_t strbuff;
    char *result;

    if(!json_is_array(json) && !json_is_object(json))
        return NULL;

    if(strbuffer_init(&strbuff))
        return NULL;

    if(do_dump(json, flags, 0, dump_to_strbuffer, (void *)&strbuff)) {
        strbuffer_close(&strbuff);
        return NULL;
    }

    if(dump_to_strbuffer("\n", 1, (void *)&strbuff)) {
        strbuffer_close(&strbuff);
        return NULL;
    }

    result = strdup(strbuffer_value(&strbuff));
    strbuffer_close(&strbuff);

    return result;
}

int json_dumpf(const json_t *json, FILE *output, unsigned long flags)
{
    if(!json_is_array(json) && !json_is_object(json))
        return -1;

    if(do_dump(json, flags, 0, dump_to_file, (void *)output))
        return -1;
    return dump_to_file("\n", 1, (void *)output);
}

int json_dump_file(const json_t *json, const char *path, unsigned long flags)
{
    int result;

    FILE *output = fopen(path, "w");
    if(!output)
        return -1;

    result = json_dumpf(json, output, flags);

    fclose(output);
    return result;
}
