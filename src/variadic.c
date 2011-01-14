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

static json_t *json_vnpack(json_error_t *error, ssize_t size, const char * const fmt, va_list *ap)
{
    json_t *root = NULL; /* root object */
    json_t *obj = NULL;

    /* Scanner variables */
    const char *tok = fmt;
    const char *etok;
    int etok_depth;

    char *key = NULL; /* Current key in an object */
    char *s;

    int line=1;
    int column=1;

    /* Skip whitespace at the beginning of the string. */
    while(size && *tok == ' ') {
        tok++;
        size--;
        column++;
    }

    if(size <= 0) {
        jsonp_error_set(error, 1, 1, "Empty format string!");
        return(NULL);
    }

    /* tok must contain either a container type, or a length-1 string for a
     * simple type. */
    if(*tok == '[')
        root = json_array();
    else if(*tok == '{')
        root = json_object();
    else
    {
        /* Simple object. Permit trailing spaces, otherwise complain. */
        if((ssize_t)strspn(tok+1, " ") < size-1)
        {
            jsonp_error_set(error, 1, 1,
                    "Expected a single object, got %i", size);
            return(NULL);
        }

        switch(*tok)
        {
            case 's': /* string */
                s = va_arg(*ap, char*);
                if(!s)
                {
                    jsonp_error_set(error, 1, 1,
                              "Refusing to handle a NULL string");
                    return(NULL);
                }
                return(json_string(s));

            case 'n': /* null */
                return(json_null());

            case 'b': /* boolean */
                obj = va_arg(*ap, int) ?
                    json_true() : json_false();
                return(obj);

            case 'i': /* integer */
                return(json_integer(va_arg(*ap, int)));

            case 'f': /* double-precision float */
                return(json_real(va_arg(*ap, double)));

            case 'O': /* a json_t object; increments refcount */
                obj = va_arg(*ap, json_t *);
                json_incref(obj);
                return(obj);

            case 'o': /* a json_t object; doesn't increment refcount */
                obj = va_arg(*ap, json_t *);
                return(obj);

            default: /* Whoops! */
                jsonp_error_set(error, 1, 1,
                        "Didn't understand format character '%c'",
                        *tok);
                return(NULL);
        }
    }

    /* Move past container opening token */
    tok++;
    column++;

    while(tok-fmt < size) {
        switch(*tok) {
            case '\n':
                line++;
                column=0;
                break;

            case ' ': /* Whitespace */
                break;

            case ',': /* Element spacer */
                if(key)
                {
                    jsonp_error_set(error, line, column,
                              "Expected KEY, got COMMA!");
                    json_decref(root);
                    return(NULL);
                }
                break;

            case ':': /* Key/value separator */
                if(!key)
                {
                    jsonp_error_set(error, line, column,
                              "Got key/value separator without "
                              "a key preceding it!");
                    json_decref(root);
                    return(NULL);
                }

                if(!json_is_object(root))
                {
                    jsonp_error_set(error, line, column,
                              "Got a key/value separator "
                              "(':') outside an object!");
                    json_decref(root);
                    return(NULL);
                }

                break;

            case ']': /* Close array or object */
            case '}':

                if(tok-fmt + (ssize_t)strspn(tok+1, " ") != size-1)
                {
                    jsonp_error_set(error, line, column,
                              "Unexpected close-bracket '%c'", *tok);
                    json_decref(root);
                    return(NULL);
                }

                if((*tok == ']' && !json_is_array(root)) ||
                   (*tok == '}' && !json_is_object(root)))
                {
                    jsonp_error_set(error, line, column,
                              "Stray close-array '%c' character", *tok);
                    json_decref(root);
                    return(NULL);
                }
                return(root);

            case '[':
            case '{':

                /* Shortcut so we don't mess up the column count in error
                 * messages */
                if(json_is_object(root) && !key)
                    goto common;

                /* Find corresponding close bracket */
                etok = tok+1;
                etok_depth = 1;
                while(etok_depth) {

                    if(!*etok || etok-fmt >= size) {
                        jsonp_error_set(error, line, column,
                                "Couldn't find matching close bracket for '%c'",
                                *tok);
                        json_decref(root);
                        return(NULL);
                    }

                    if(*tok==*etok)
                        etok_depth++;
                    else if(*tok=='[' && *etok==']') {
                        etok_depth--;
                        break;
                    } else if(*tok=='{' && *etok=='}') {
                        etok_depth--;
                        break;
                    }

                    etok++;
                }

                /* Recurse */
                obj = json_vnpack(error, etok-tok+1, tok, ap);
                if(!obj) {
                    /* error should already be set */
                    error->column += column-1;
                    error->line += line-1;
                    json_decref(root);
                    return(NULL);
                }
                column += etok-tok;
                tok = etok;
                goto common;

            case 's':
                /* Handle strings specially, since they're used for both keys
                 * and values */
                s = va_arg(*ap, char*);

                if(!s)
                {
                    jsonp_error_set(error, line, column,
                              "Refusing to handle a NULL string");
                    json_decref(root);
                    return(NULL);
                }

                if(json_is_object(root) && !key)
                {
                    /* It's a key */
                    key = s;
                    break;
                }

                obj = json_string(s);
                goto common;

        default:
                obj = json_vnpack(error, 1, tok, ap);
                if(!obj) {
                    json_decref(root);
                    return(NULL);
                }

common:
                /* Add to container */
                if(json_is_object(root)) {
                    if(!key)
                    {
                        jsonp_error_set(error, line, column,
                              "Expected key, got identifier '%c'!", *tok);
                        json_decref(root);
                        return(NULL);
                    }

                    json_object_set_new(root, key, obj);
                    key = NULL;
                }
                else
                {
                    json_array_append_new(root, obj);
                }
                break;
        }
        tok++;
        column++;
    }

    /* Whoops -- we didn't match the close bracket! */
    jsonp_error_set(error, line, column, "Missing close array or object!");
    json_decref(root);
    return(NULL);
}

static int json_vnunpack(json_t *root, json_error_t *error, ssize_t size, const char *fmt, va_list *ap)
{

    int rv=0; /* Return value */
    int line = 1; /* Line number */
    int column = 1; /* Column */

    /* Position markers for arrays or objects */
    int array_index = 0;
    char *key = NULL;

    const char **s;

    /* Scanner variables */
    const char *tok = fmt;
    const char *etok;
    int etok_depth;

    json_t *obj;

    /* If we're successful, we need to know if the number of arguments
     * provided matches the number of JSON objects.  We can do this by
     * counting the elements in every array or object we open up, and
     * decrementing the count as we visit their children. */
    int unvisited = 0;

    /* Skip whitespace at the beginning of the string. */
    while(size && *tok == ' ') {
        tok++;
        size--;
        column++;
    }

    if(size <= 0) {
        jsonp_error_set(error, 1, 1, "Empty format string!");
        return(-2);
    }

    /* tok must contain either a container type, or a length-1 string for a
     * simple type. */
    if(*tok != '[' && *tok != '{')
    {
        /* Simple object. Permit trailing spaces, otherwise complain. */
        if((ssize_t)strspn(tok+1, " ") < size-1)
        {
            jsonp_error_set(error, 1, 1,
                    "Expected a single object, got %i", size);
            return(-1);
        }

        switch(*tok)
        {
            case 's':
                if(!json_is_string(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a string.",
                            json_typeof(root));
                    return(-2);
                }
                s = va_arg(*ap, const char **);
                if(!s) {
                    jsonp_error_set(error, line, column, "Passed a NULL string pointer!");
                    return(-2);
                }
                *s = json_string_value(root);
                return(0);

            case 'i':
                if(!json_is_integer(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't an integer.",
                            json_typeof(root));
                    return(-2);
                }
                *va_arg(*ap, int*) = json_integer_value(root);
                return(0);

            case 'b':
                if(!json_is_boolean(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a boolean.",
                            json_typeof(root));
                    return(-2);
                }
                *va_arg(*ap, int*) = json_is_true(root);
                return(0);

            case 'f':
                if(!json_is_number(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a real.",
                            json_typeof(root));
                    return(-2);
                }
                *va_arg(*ap, double*) = json_number_value(root);
                return(0);

            case 'O':
                json_incref(root);
                /* Fall through */

            case 'o':
                *va_arg(*ap, json_t**) = root;
                return(0);

            case 'n':
                /* Don't actually assign anything; we're just happy
                 * the null turned up as promised in the format
                 * string. */
                return(0);

            default:
                jsonp_error_set(error, line, column,
                        "Unknown format character '%c'", *tok);
                return(-1);
        }
    }

    /* Move past container opening token */
    tok++;

    while(tok-fmt < size) {
        switch(*tok) {
            case '\n':
                line++;
                column=0;
                break;

            case ' ': /* Whitespace */
                break;

            case ',': /* Element spacer */
                if(key)
                {
                    jsonp_error_set(error, line, column,
                              "Expected KEY, got COMMA!");
                    return(-2);
                }
                break;

            case ':': /* Key/value separator */
                if(!key)
                {
                    jsonp_error_set(error, line, column,
                              "Got key/value separator without "
                              "a key preceding it!");
                    return(-2);
                }

                if(!json_is_object(root))
                {
                    jsonp_error_set(error, line, column,
                              "Got a key/value separator "
                              "(':') outside an object!");
                    return(-2);
                }

                break;

            case ']': /* Close array or object */
            case '}':

                if(tok-fmt + (ssize_t)strspn(tok+1, " ") != size-1)
                {
                    jsonp_error_set(error, line, column,
                              "Unexpected close-bracket '%c'", *tok);
                    return(-2);
                }

                if((*tok == ']' && !json_is_array(root)) ||
                   (*tok == '}' && !json_is_object(root)))
                {
                    jsonp_error_set(error, line, column,
                              "Stray close-array '%c' character", *tok);
                    return(-2);
                }
                return(unvisited);

            case '[':
            case '{':

                /* Find corresponding close bracket */
                etok = tok+1;
                etok_depth = 1;
                while(etok_depth) {

                    if(!*etok || etok-fmt >= size) {
                        jsonp_error_set(error, line, column,
                                "Couldn't find matching close bracket for '%c'",
                                *tok);
                        return(-2);
                    }

                    if(*tok==*etok)
                        etok_depth++;
                    else if(*tok=='[' && *etok==']') {
                        etok_depth--;
                        break;
                    } else if(*tok=='{' && *etok=='}') {
                        etok_depth--;
                        break;
                    }

                    etok++;
                }

                /* Recurse */
                if(json_is_array(root)) {
                    rv = json_vnunpack(json_object_get(root, key),
                            error, etok-tok+1, tok, ap);
                } else {
                    rv = json_vnunpack(json_array_get(root, array_index++),
                            error, etok-tok+1, tok, ap);
                }

                if(rv < 0) {
                    /* error should already be set */
                    error->column += column-1;
                    error->line += line-1;
                    return(rv);
                }

                unvisited += rv;
                column += etok-tok;
                tok = etok;
                break;

            case 's':
                /* Handle strings specially, since they're used for both keys
                 * and values */

                if(json_is_object(root) && !key)
                {
                    /* It's a key */
                    key = va_arg(*ap, char*);
                    printf("Got key '%s'\n", key);

                    if(!key)
                    {
                        jsonp_error_set(error, line, column,
                                  "Refusing to handle a NULL key");
                        return(-2);
                    }
                    break;
                }

                /* Fall through */

            default:

                /* Fetch the element from the JSON container */
                if(json_is_object(root))
                    obj = json_object_get(root, key);
                else
                    obj = json_array_get(root, array_index++);

                if(!obj) {
                    jsonp_error_set(error, line, column,
                            "Array/object entry didn't exist!");
                    return(-1);
                }

                rv = json_vnunpack(obj, error, 1, tok, ap);
                if(rv != 0)
                    return(rv);

                break;
        }
        tok++;
        column++;
    }

    /* Whoops -- we didn't match the close bracket! */
    jsonp_error_set(error, line, column, "Missing close array or object!");
    return(-2);
}

json_t *json_pack(json_error_t *error, const char *fmt, ...)
{
    va_list ap;
    json_t *obj;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string!");
        return(NULL);
    }

    va_start(ap, fmt);
    obj = json_vnpack(error, strlen(fmt), fmt, &ap);
    va_end(ap);

    return(obj);
}

int json_unpack(json_t *root, json_error_t *error, const char *fmt, ...)
{
    va_list ap;
    int rv;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string!");
        return(-2);;
    }

    va_start(ap, fmt);
    rv = json_vnunpack(root, error, strlen(fmt), fmt, &ap);
    va_end(ap);

    return(rv);
}

/* vim: ts=4:expandtab:sw=4
 */
