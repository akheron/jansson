/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2011 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <jansson.h>
#include "jansson_private.h"

typedef struct {
    const char *fmt;
    char token;
    json_error_t *error;
    int line;
    int column;
} scanner_t;

static void next_token(scanner_t *s)
{
    const char *t = s->fmt;
    s->column++;

    /* skip space and ignored chars */
    while(*t == ' ' || *t == '\t' || *t == '\n' || *t == ',' || *t == ':') {
        if(*t == '\n') {
            s->line++;
            s->column = 1;
        }
        else
            s->column++;

        t++;
    }

    s->token = *t;

    t++;
    s->fmt = t;
}

static void set_error(scanner_t *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    jsonp_error_vset(s->error, s->line, s->column, fmt, ap);
    va_end(ap);
}

static json_t *pack(scanner_t *s, va_list *ap);

static json_t *pack_object(scanner_t *s, va_list *ap)
{
    json_t *object = json_object();
    next_token(s);

    while(s->token != '}') {
        const char *key;
        json_t *value;

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            goto error;
        }

        if(s->token != 's') {
            set_error(s, "Expected format 's', got '%c'\n", *s->fmt);
            goto error;
        }

        key = va_arg(*ap, const char *);
        if(!key) {
            set_error(s, "NULL object key");
            goto error;
        }

        next_token(s);

        value = pack(s, ap);
        if(!value)
            goto error;

        if(json_object_set_new(object, key, value)) {
            set_error(s, "Unable to add key \"%s\"", key);
            goto error;
        }

        next_token(s);
    }

    return object;

error:
    json_decref(object);
    return NULL;
}

static json_t *pack_array(scanner_t *s, va_list *ap)
{
    json_t *array = json_array();
    next_token(s);

    while(s->token != ']') {
        json_t *value;

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            goto error;
        }

        value = pack(s, ap);
        if(!value)
            goto error;

        if(json_array_append_new(array, value)) {
            set_error(s, "Unable to append to array");
            goto error;
        }

        next_token(s);
    }
    return array;

error:
    json_decref(array);
    return NULL;
}

static json_t *pack(scanner_t *s, va_list *ap)
{
    switch(s->token) {
        case '{':
            return pack_object(s, ap);

        case '[':
            return pack_array(s, ap);

        case 's': /* string */
        {
            const char *str = va_arg(*ap, const char *);
            if(!str)
            {
                set_error(s, "NULL string");
                return NULL;
            }
            return json_string(str);
        }

        case 'n': /* null */
            return json_null();

        case 'b': /* boolean */
            return va_arg(*ap, int) ? json_true() : json_false();

        case 'i': /* integer */
            return json_integer(va_arg(*ap, int));

        case 'f': /* double-precision float */
            return json_real(va_arg(*ap, double));

        case 'O': /* a json_t object; increments refcount */
            return json_incref(va_arg(*ap, json_t *));

        case 'o': /* a json_t object; doesn't increment refcount */
            return va_arg(*ap, json_t *);

        default: /* Whoops! */
            set_error(s, "Unrecognized format character '%c'", s->token);
            return NULL;
    }
}

static int json_vnunpack(json_t *root, json_error_t *error, ssize_t size, const char *fmt, va_list *ap)
{

    int rv = 0; /* Return value */
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
        return -2;
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
            return -1;
        }

        switch(*tok)
        {
            case 's':
                if(!json_is_string(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a string.",
                            json_typeof(root));
                    return -2;
                }
                s = va_arg(*ap, const char **);
                if(!s) {
                    jsonp_error_set(error, line, column, "Passed a NULL string pointer!");
                    return -2;
                }
                *s = json_string_value(root);
                return 0;

            case 'i':
                if(!json_is_integer(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't an integer.",
                            json_typeof(root));
                    return -2;
                }
                *va_arg(*ap, int*) = json_integer_value(root);
                return 0;

            case 'b':
                if(!json_is_boolean(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a boolean.",
                            json_typeof(root));
                    return -2;
                }
                *va_arg(*ap, int*) = json_is_true(root);
                return 0;

            case 'f':
                if(!json_is_number(root))
                {
                    jsonp_error_set(error, line, column,
                            "Type mismatch! Object (%i) wasn't a real.",
                            json_typeof(root));
                    return -2;
                }
                *va_arg(*ap, double*) = json_number_value(root);
                return 0;

            case 'O':
                json_incref(root);
                /* Fall through */

            case 'o':
                *va_arg(*ap, json_t**) = root;
                return 0;

            case 'n':
                /* Don't actually assign anything; we're just happy
                 * the null turned up as promised in the format
                 * string. */
                return 0;

            default:
                jsonp_error_set(error, line, column,
                        "Unknown format character '%c'", *tok);
                return -1;
        }
    }

    /* Move past container opening token */
    tok++;

    while(tok-fmt < size) {
        switch(*tok) {
            case '\n':
                line++;
                column = 0;
                break;

            case ' ': /* Whitespace */
                break;

            case ',': /* Element spacer */
                if(key)
                {
                    jsonp_error_set(error, line, column,
                              "Expected KEY, got COMMA!");
                    return -2;
                }
                break;

            case ':': /* Key/value separator */
                if(!key)
                {
                    jsonp_error_set(error, line, column,
                              "Got key/value separator without "
                              "a key preceding it!");
                    return -2;
                }

                if(!json_is_object(root))
                {
                    jsonp_error_set(error, line, column,
                              "Got a key/value separator "
                              "(':') outside an object!");
                    return -2;
                }

                break;

            case ']': /* Close array or object */
            case '}':

                if(tok-fmt + (ssize_t)strspn(tok+1, " ") != size-1)
                {
                    jsonp_error_set(error, line, column,
                              "Unexpected close-bracket '%c'", *tok);
                    return -2;
                }

                if((*tok == ']' && !json_is_array(root)) ||
                   (*tok == '}' && !json_is_object(root)))
                {
                    jsonp_error_set(error, line, column,
                              "Stray close-array '%c' character", *tok);
                    return -2;
                }
                return unvisited;

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
                        return -2;
                    }

                    if(*tok == *etok)
                        etok_depth++;
                    else if(*tok == '[' && *etok == ']') {
                        etok_depth--;
                        break;
                    } else if(*tok == '{' && *etok == '}') {
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
                    return rv;
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
                    key = va_arg(*ap, char *);

                    if(!key)
                    {
                        jsonp_error_set(error, line, column,
                                  "Refusing to handle a NULL key");
                        return -2;
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
                    return -1;
                }

                rv = json_vnunpack(obj, error, 1, tok, ap);
                if(rv != 0)
                    return rv;

                break;
        }
        tok++;
        column++;
    }

    /* Whoops -- we didn't match the close bracket! */
    jsonp_error_set(error, line, column, "Missing close array or object!");
    return -2;
}

json_t *json_pack(json_error_t *error, const char *fmt, ...)
{
    scanner_t s;
    json_t *value;
    va_list ap;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string!");
        return NULL;
    }

    s.error = error;
    s.fmt = fmt;
    s.line = 1;
    s.column = 0;

    next_token(&s);

    va_start(ap, fmt);
    value = pack(&s, &ap);
    va_end(ap);

    next_token(&s);
    if(s.token) {
        set_error(&s, "Garbage after format string");
        return NULL;
    }

    return value;
}

int json_unpack(json_t *root, json_error_t *error, const char *fmt, ...)
{
    va_list ap;
    int rv;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string!");
        return -2;;
    }

    va_start(ap, fmt);
    rv = json_vnunpack(root, error, strlen(fmt), fmt, &ap);
    va_end(ap);

    return rv;
}
