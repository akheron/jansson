/*
 * Copyright (c) 2009, 2010 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <jansson.h>
#include "jansson_private.h"

json_t *json_pack(json_error_t *error, const char *fmt, ...) {
    int fmt_length = strlen(fmt);
    va_list ap;

    /* Keep a stack of containers (lists and objects) */
    int depth = 0;
    json_t **stack = NULL;

    /* Keep a list of objects we create in case of error */
    int free_count = 0;
    json_t **free_list = NULL;

    json_t *cur = NULL; /* Current container */
    json_t *root = NULL; /* root object */
    json_t *obj = NULL;

    char *key = NULL; /* Current key in an object */
    char *s;

    int line = 1;

    /* Allocation provisioned for worst case */
    stack = calloc(fmt_length, sizeof(json_t *));
    free_list = calloc(fmt_length, sizeof(json_t *));

    jsonp_error_init(error, "");

    if(!stack || !free_list)
        goto out;

    va_start(ap, fmt);
    while(*fmt) {
        switch(*fmt) {
            case '\n':
                line++;
                break;

            case ' ': /* Whitespace */
                break;

            case ',': /* Element spacer */
                if(!root)
                {
                    jsonp_error_set(error, line, -1,
                              "Unexpected COMMA precedes root element!");
                    root = NULL;
                    goto out;
                }

                if(!cur)
                {
                    jsonp_error_set(error, line, -1,
                              "Unexpected COMMA outside a list or object!");
                    root = NULL;
                    goto out;
                }

                if(key)
                {
                    jsonp_error_set(error, line, -1,
                              "Expected KEY, got COMMA!");
                    root = NULL;
                    goto out;
                }
                break;

            case ':': /* Key/value separator */
                if(!key)
                {
                    jsonp_error_set(error, line, -1,
                              "Got key/value separator without "
                              "a key preceding it!");
                    root = NULL;
                    goto out;
                }

                if(!json_is_object(cur))
                {
                    jsonp_error_set(error, line, -1,
                              "Got a key/value separator "
                              "(':') outside an object!");
                    root = NULL;
                    goto out;
                }

                break;

            case ']': /* Close array or object */
            case '}':

                if(key)
                {
                    jsonp_error_set(error, line, -1,
                              "OBJECT or ARRAY ended with an "
                              "incomplete key/value pair!");
                    root = NULL;
                    goto out;
                }

                if(depth <= 0)
                {
                    jsonp_error_set(error, line, -1,
                              "Too many close-brackets '%c'", *fmt);
                    root = NULL;
                    goto out;
                }

                if(*fmt == ']' && !json_is_array(cur))
                {
                    jsonp_error_set(error, line, -1,
                              "Stray close-array ']' character");
                    root = NULL;
                    goto out;
                }

                if(*fmt == '}' && !json_is_object(cur))
                {
                    jsonp_error_set(error, line, -1,
                              "Stray close-object '}' character");
                    root = NULL;
                    goto out;
                }

                cur = stack[--depth];
                break;

            case '[':
                obj = json_array();
                goto obj_common;

            case '{':
                obj = json_object();
                goto obj_common;

            case 's': /* string */
                s = va_arg(ap, char*);

                if(!s)
                {
                    jsonp_error_set(error, line, -1,
                              "Refusing to handle a NULL string");
                    root = NULL;
                    goto out;
                }

                if(json_is_object(cur) && !key)
                {
                    /* It's a key */
                    key = s;
                    break;
                }

                obj = json_string(s);
                goto obj_common;

            case 'n': /* null */
                obj = json_null();
                goto obj_common;

            case 'b': /* boolean */
                obj = va_arg(ap, int) ?
                    json_true() : json_false();
                goto obj_common;

            case 'i': /* integer */
                obj = json_integer(va_arg(ap, int));
                goto obj_common;

            case 'f': /* double-precision float */
                obj = json_real(va_arg(ap, double));
                goto obj_common;

            case 'O': /* a json_t object; increments refcount */
                obj = va_arg(ap, json_t *);
                json_incref(obj);
                goto obj_common;

            case 'o': /* a json_t object; doesn't increment refcount */
                obj = va_arg(ap, json_t *);
                goto obj_common;

obj_common:     free_list[free_count++] = obj;

                /* Root this object to its parent */
                if(json_is_object(cur)) {
                    if(!key)
                    {
                        jsonp_error_set(error, line, -1,
                              "Expected key, got identifier '%c'!", *fmt);
                        root = NULL;
                        goto out;
                    }

                    json_object_set_new(cur, key, obj);
                    key = NULL;
                }
                else if(json_is_array(cur))
                {
                    json_array_append_new(cur, obj);
                }
                else if(!root)
                {
                    printf("Rooting\n");
                    root = obj;
                }
                else
                {
                    jsonp_error_set(error, line, -1,
                              "Can't figure out where to attach "
                              "'%c' object!", *fmt);
                    root = NULL;
                    goto out;
                }

                /* If it was a container ('[' or '{'), descend on the stack */
                if(json_is_array(obj) || json_is_object(obj))
                {
                    stack[depth++] = cur;
                    cur = obj;
                }

                break;
        }
        fmt++;
    }
    va_end(ap);

    if(depth != 0) {
        jsonp_error_set(error, line, -1,
                "Missing object or array close-brackets in format string");
        root = NULL;
        goto out;
    }

    /* Success: don't free everything we just built! */
    free_count = 0;

out:
    while(free_count)
        json_decref(free_list[--free_count]);

    if(free_list)
        free(free_list);

    if(stack)
        free(stack);

    return(root);
}

int json_unpack(json_t *root, json_error_t *error, const char *fmt, ...) {
    va_list ap;

    int rv=0; /* Return value */
    int line = 1; /* Line number */

    /* Keep a stack of containers (lists and objects) */
    int depth = 0;
    json_t **stack;

    int array_index = 0;
    char *key = NULL; /* Current key in an object */

    json_t *cur = NULL; /* Current container */
    json_t *obj = NULL;

    int fmt_length = strlen(fmt);

    jsonp_error_init(error, "");

    /* Allocation provisioned for worst case */
    stack = calloc(fmt_length, sizeof(json_t *));
    if(!stack)
    {
        jsonp_error_set(error, line, -1, "Out of memory!");
        rv = -1;
        goto out;
    }

    /* Even if we're successful, we need to know if the number of
     * arguments provided matches the number of JSON objects.
     * We can do this by counting the elements in every array or
     * object we open up, and decrementing the count as we visit
     * their children. */
    int unvisited = 0;

    va_start(ap, fmt);
    while(*fmt)
    {
        switch(*fmt)
        {
            case ' ': /* Whitespace */
                break;

            case '\n': /* Line break */
                line++;
                break;

            case ',': /* Element spacer */

                if(!cur)
                {
                    jsonp_error_set(error, line, -1,
                              "Unexpected COMMA outside a list or object!");
                    rv = -1;
                    goto out;
                }

                if(key)
                {
                    jsonp_error_set(error, line, -1,
                              "Expected KEY, got COMMA!");
                    rv = -1;
                    goto out;
                }
                break;

            case ':': /* Key/value separator */
                if(!json_is_object(cur) || !key)
                {
                    jsonp_error_set(error, line, -1, "Unexpected ':'");
                    rv = -1;
                    goto out;
                }
                break;

            case '[':
            case '{':
                /* Fetch object */
                if(!cur)
                {
                    obj = root;
                }
                else if(json_is_object(cur))
                {
                    if(!key)
                    {
                        jsonp_error_set(error, line, -1,
                              "Objects can't be keys");
                        rv = -1;
                        goto out;
                    }
                    obj = json_object_get(cur, key);
                    unvisited--;
                    key = NULL;
                }
                else if(json_is_array(cur))
                {
                    obj = json_array_get(cur, array_index);
                    unvisited--;
                    array_index++;
                }
                else
                {
                    assert(0);
                }

                /* Make sure we got what we expected */
                if(*fmt=='{' && !json_is_object(obj))
                {
                    rv = -2;
                    goto out;
                }

                if(*fmt=='[' && !json_is_array(obj))
                {
                    rv = -2;
                    goto out;
                }

                unvisited += json_is_object(obj) ?
                    json_object_size(obj) :
                    json_array_size(obj);

                /* Descend */
                stack[depth++] = cur;
                cur = obj;

                key = NULL;

                break;


            case ']':
            case '}':

                if(json_is_array(cur) && *fmt!=']')
                {
                    jsonp_error_set(error, line, -1, "Missing ']'");
                    rv = -1;
                    goto out;
                }

                if(json_is_object(cur) && *fmt!='}')
                {
                    jsonp_error_set(error, line, -1, "Missing '}'");
                    rv = -1;
                    goto out;
                }

                if(key)
                {
                    jsonp_error_set(error, line, -1, "Unexpected '%c'", *fmt);
                    rv = -1;
                    goto out;
                }

                if(depth <= 0)
                {
                    jsonp_error_set(error, line, -1, "Unexpected '%c'", *fmt);
                    rv = -1;
                    goto out;
                }

                cur = stack[--depth];

                break;

            case 's':
                if(!key && json_is_object(cur))
                {
                    /* constant string for key */
                    key = va_arg(ap, char*);
                    break;
                }
                /* fall through */

            case 'i': /* integer */
            case 'f': /* double-precision float */
            case 'O': /* a json_t object; increments refcount */
            case 'o': /* a json_t object; borrowed reference */
            case 'b': /* boolean */
            case 'n': /* null */

                /* Fetch object */
                if(!cur)
                {
                    obj = root;
                }
                else if(json_is_object(cur))
                {
                    if(!key)
                    {
                        jsonp_error_set(error, line, -1,
                                  "Only strings may be used as keys!");
                        rv = -1;
                        goto out;
                    }

                    obj = json_object_get(cur, key);
                    unvisited--;
                    key = NULL;
                }
                else if(json_is_array(cur))
                {
                    obj = json_array_get(cur, array_index);
                    unvisited--;
                    array_index++;
                }
                else
                {
                    jsonp_error_set(error, line, -1,
                              "Unsure how to retrieve JSON object '%c'",
                              *fmt);
                    rv = -1;
                    goto out;
                }

                switch(*fmt)
                {
                    case 's':
                        if(!json_is_string(obj))
                        {
                            jsonp_error_set(error, line, -1,
                                    "Type mismatch! Object wasn't a string.");
                            rv = -2;
                            goto out;
                        }
                        *va_arg(ap, const char**) = json_string_value(obj);
                        break;

                    case 'i':
                        if(!json_is_integer(obj))
                        {
                            jsonp_error_set(error, line, -1,
                                    "Type mismatch! Object wasn't an integer.");
                            rv = -2;
                            goto out;
                        }
                        *va_arg(ap, int*) = json_integer_value(obj);
                        break;

                    case 'b':
                        if(!json_is_boolean(obj))
                        {
                            jsonp_error_set(error, line, -1,
                                    "Type mismatch! Object wasn't a boolean.");
                            rv = -2;
                            goto out;
                        }
                        *va_arg(ap, int*) = json_is_true(obj);
                        break;

                    case 'f':
                        if(!json_is_number(obj))
                        {
                            jsonp_error_set(error, line, -1,
                                    "Type mismatch! Object wasn't a real.");
                            rv = -2;
                            goto out;
                        }
                        *va_arg(ap, double*) = json_number_value(obj);
                        break;

                    case 'O':
                        json_incref(obj);
                        /* Fall through */

                    case 'o':
                        *va_arg(ap, json_t**) = obj;
                        break;

                    case 'n':
                        /* Don't actually assign anything; we're just happy
                         * the null turned up as promised in the format
                         * string. */
                        break;

                    default:
                        jsonp_error_set(error, line, -1,
                                "Unknown format character '%c'", *fmt);
                        rv = -1;
                        goto out;
                }
        }
        fmt++;
    }

    /* Return 0 if everything was matched; otherwise the number of JSON
     * objects we didn't get to. */
    rv = unvisited;

out:
    va_end(ap);

    if(stack)
        free(stack);

    return(rv);
}

/* vim: ts=4:expandtab:sw=4
 */
