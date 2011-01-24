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

static int unpack(scanner_t *s, json_t *root, va_list *ap);

static int unpack_object(scanner_t *s, json_t *root, va_list *ap)
{
    if(!json_is_object(root)) {
        set_error(s, "Expected object, got %i", json_typeof(root));
        return -1;
    }
    next_token(s);

    while(s->token != '}') {
        const char *key;
        json_t *value;

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            return -1;
        }

        if(s->token != 's') {
            set_error(s, "Expected format 's', got '%c'\n", *s->fmt);
            return -1;
        }

        key = va_arg(*ap, const char *);
        if(!key) {
            set_error(s, "NULL object key");
            return -1;
        }

        next_token(s);

        value = json_object_get(root, key);
        if(unpack(s, value, ap))
            return -1;

        next_token(s);
    }

    return 0;
}

static int unpack_array(scanner_t *s, json_t *root, va_list *ap)
{
    size_t i = 0;

    if(!json_is_array(root)) {
        set_error(s, "Expected array, got %d", json_typeof(root));
        return -1;
    }
    next_token(s);

    while(s->token != ']') {
        json_t *value;

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            return -1;
        }

        value = json_array_get(root, i);
        if(!value) {
            set_error(s, "Array index %lu out of range", (unsigned long)i);
            return -1;
        }

        if(unpack(s, value, ap))
            return -1;

        next_token(s);
        i++;
    }

    if(i != json_array_size(root)) {
        long diff = (long)json_array_size(root) - (long)i;
        set_error(s, "%li array items were not upacked", diff);
        return -1;
    }

    return 0;
}

static int unpack(scanner_t *s, json_t *root, va_list *ap)
{
    switch(s->token)
    {
        case '{':
            return unpack_object(s, root, ap);

        case '[':
            return unpack_array(s, root, ap);

        case 's':
        {
            const char **str;

            if(!json_is_string(root))
            {
                set_error(s, "Type mismatch! Object (%i) wasn't a string.",
                          json_typeof(root));
                return -1;
            }

            str = va_arg(*ap, const char **);
            if(!str) {
                set_error(s, "Passed a NULL string pointer!");
                return -1;
            }

            *str = json_string_value(root);
            return 0;
        }

        case 'i':
            if(!json_is_integer(root))
            {
                set_error(s, "Type mismatch! Object (%i) wasn't an integer.",
                      json_typeof(root));
                return -1;
            }
            *va_arg(*ap, int*) = json_integer_value(root);
            return 0;

        case 'b':
            if(!json_is_boolean(root))
            {
                set_error(s, "Type mismatch! Object (%i) wasn't a boolean.",
                      json_typeof(root));
                return -1;
            }
            *va_arg(*ap, int*) = json_is_true(root);
            return 0;

        case 'f':
            if(!json_is_number(root))
            {
                set_error(s, "Type mismatch! Object (%i) wasn't a real.",
                      json_typeof(root));
                return -1;
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
            /* Don't assign, just validate */
            if(!json_is_null(root))
            {
                set_error(s, "Type mismatch! Object (%i) wasn't null.",
                      json_typeof(root));
                return -1;
            }
            return 0;

        default:
            set_error(s, "Unknown format character '%c'", s->token);
            return -1;
    }
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
    scanner_t s;
    va_list ap;
    int result;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string!");
        return -1;
    }

    s.error = error;
    s.fmt = fmt;
    s.line = 1;
    s.column = 0;

    next_token(&s);

    va_start(ap, fmt);
    result = unpack(&s, root, &ap);
    va_end(ap);

    if(result)
        return -1;

    next_token(&s);
    if(s.token) {
        set_error(&s, "Garbage after format string");
        return -1;
    }

    return 0;
}
