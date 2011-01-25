/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2011 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include "jansson_private.h"

typedef struct {
    const char *fmt;
    char token;
    json_error_t *error;
    size_t flags;
    int line;
    int column;
} scanner_t;

static const char *type_names[] = {
    "object",
    "array",
    "string",
    "integer",
    "real",
    "true",
    "false",
    "null"
};

#define type_name(x) type_names[json_typeof(x)]

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
            if(!str) {
                set_error(s, "NULL string");
                return NULL;
            }
            return json_string(str);
        }

        case 'n': /* null */
            return json_null();

        case 'b': /* boolean */
            return va_arg(*ap, int) ? json_true() : json_false();

        case 'i': /* integer from int */
            return json_integer(va_arg(*ap, int));

        case 'I': /* integer from json_int_t */
            return json_integer(va_arg(*ap, json_int_t));

        case 'f': /* real */
            return json_real(va_arg(*ap, double));

        case 'O': /* a json_t object; increments refcount */
            return json_incref(va_arg(*ap, json_t *));

        case 'o': /* a json_t object; doesn't increment refcount */
            return va_arg(*ap, json_t *);

        default:
            set_error(s, "Unrecognized format character '%c'", s->token);
            return NULL;
    }
}

static int unpack(scanner_t *s, json_t *root, va_list *ap);

static int unpack_object(scanner_t *s, json_t *root, va_list *ap)
{
    int ret = -1;
    int wildcard = 0;

    /* Use a set (emulated by a hashtable) to check that all object
       keys are accessed. Checking that the correct number of keys
       were accessed is not enough, as the same key can be unpacked
       multiple times.
    */
    hashtable_t *key_set;

    if(!(s->flags & JSON_UNPACK_ONLY)) {
        key_set = hashtable_create(jsonp_hash_key, jsonp_key_equal, NULL, NULL);
        if(!key_set) {
            set_error(s, "Out of memory");
            return -1;
        }
    }

    if(!json_is_object(root)) {
        set_error(s, "Expected object, got %s", type_name(root));
        goto error;
    }
    next_token(s);

    while(s->token != '}') {
        const char *key;
        json_t *value;

        if(wildcard) {
            set_error(s, "Expected '}' after '*', got '%c'", s->token);
            goto error;
        }

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            goto error;
        }

        if(s->token == '*') {
            wildcard = 1;
            next_token(s);
            continue;
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

        value = json_object_get(root, key);
        if(unpack(s, value, ap))
            goto error;

        if(!(s->flags & JSON_UNPACK_ONLY))
            hashtable_set(key_set, (void *)key, NULL);

        next_token(s);
    }

    if(s->flags & JSON_UNPACK_ONLY)
        wildcard = 1;

    if(!wildcard && key_set->size != json_object_size(root)) {
        long diff = (long)json_object_size(root) - (long)key_set->size;
        set_error(s, "%li object items left unpacked", diff);
        goto error;
    }

    ret = 0;

error:
    if(!(s->flags & JSON_UNPACK_ONLY))
        hashtable_destroy(key_set);

    return ret;
}

static int unpack_array(scanner_t *s, json_t *root, va_list *ap)
{
    size_t i = 0;
    int wildcard = 0;

    if(!json_is_array(root)) {
        set_error(s, "Expected array, got %s", type_name(root));
        return -1;
    }
    next_token(s);

    while(s->token != ']') {
        json_t *value;

        if(wildcard) {
            set_error(s, "Expected ']' after '*', got '%c'", s->token);
            return -1;
        }

        if(!s->token) {
            set_error(s, "Unexpected end of format string");
            return -1;
        }

        if(s->token == '*') {
            wildcard = 1;
            next_token(s);
            continue;
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

    if(s->flags & JSON_UNPACK_ONLY)
        wildcard = 1;

    if(!wildcard && i != json_array_size(root)) {
        long diff = (long)json_array_size(root) - (long)i;
        set_error(s, "%li array items left upacked", diff);
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
            if(!json_is_string(root)) {
                set_error(s, "Expected string, got %s", type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY)) {
                const char **str;

                str = va_arg(*ap, const char **);
                if(!str) {
                    set_error(s, "NULL string");
                    return -1;
                }

                *str = json_string_value(root);
            }
            return 0;

        case 'i':
            if(!json_is_integer(root)) {
                set_error(s, "Expected integer, got %s", type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, int*) = json_integer_value(root);

            return 0;

        case 'I':
            if(!json_is_integer(root)) {
                set_error(s, "Expected integer, got %s", type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, json_int_t*) = json_integer_value(root);

            return 0;

        case 'b':
            if(!json_is_boolean(root)) {
                set_error(s, "Expected true or false, got %s", type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, int*) = json_is_true(root);

            return 0;

        case 'f':
            if(!json_is_real(root)) {
                set_error(s, "Expected real, got %s", type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, double*) = json_real_value(root);

            return 0;

        case 'F':
            if(!json_is_number(root)) {
                set_error(s, "Expected real or integer, got %s",
                          type_name(root));
                return -1;
            }

            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, double*) = json_number_value(root);

            return 0;

        case 'O':
            if(!(s->flags & JSON_VALIDATE_ONLY))
                json_incref(root);
            /* Fall through */

        case 'o':
            if(!(s->flags & JSON_VALIDATE_ONLY))
                *va_arg(*ap, json_t**) = root;

            return 0;

        case 'n':
            /* Never assign, just validate */
            if(!json_is_null(root)) {
                set_error(s, "Expected null, got %s", type_name(root));
                return -1;
            }
            return 0;

        default:
            set_error(s, "Unknown format character '%c'", s->token);
            return -1;
    }
}

json_t *json_vpack_ex(json_error_t *error, size_t flags,
                      const char *fmt, va_list ap)
{
    scanner_t s;
    json_t *value;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string");
        return NULL;
    }

    s.error = error;
    s.flags = flags;
    s.fmt = fmt;
    s.line = 1;
    s.column = 0;

    next_token(&s);
    value = pack(&s, &ap);

    next_token(&s);
    if(s.token) {
        json_decref(value);
        set_error(&s, "Garbage after format string");
        return NULL;
    }

    return value;
}

json_t *json_pack_ex(json_error_t *error, size_t flags, const char *fmt, ...)
{
    json_t *value;
    va_list ap;

    va_start(ap, fmt);
    value = json_vpack_ex(error, flags, fmt, ap);
    va_end(ap);

    return value;
}

json_t *json_pack(const char *fmt, ...)
{
    json_t *value;
    va_list ap;

    va_start(ap, fmt);
    value = json_vpack_ex(NULL, 0, fmt, ap);
    va_end(ap);

    return value;
}

int json_vunpack_ex(json_t *root, json_error_t *error, size_t flags,
                    const char *fmt, va_list ap)
{
    scanner_t s;

    jsonp_error_init(error, "");

    if(!fmt || !*fmt) {
        jsonp_error_set(error, 1, 1, "Null or empty format string");
        return -1;
    }

    s.error = error;
    s.flags = flags;
    s.fmt = fmt;
    s.line = 1;
    s.column = 0;

    next_token(&s);

    if(unpack(&s, root, &ap))
        return -1;

    next_token(&s);
    if(s.token) {
        set_error(&s, "Garbage after format string");
        return -1;
    }

    return 0;
}

int json_unpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = json_vunpack_ex(root, error, flags, fmt, ap);
    va_end(ap);

    return ret;
}

int json_unpack(json_t *root, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = json_vunpack_ex(root, NULL, 0, fmt, ap);
    va_end(ap);

    return ret;
}
