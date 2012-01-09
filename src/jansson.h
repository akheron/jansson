/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_H
#define JANSSON_H

#include <stdio.h>
#include <stdlib.h>  /* for size_t */
#include <stdarg.h>


#if defined (jansson_EXPORTS)/* it's defined by cmake while building jansson */
# if defined (_MSC_VER)
#  define EXPORT __declspec(dllexport)
# else /* gcc requires explicit exports with -fvisibility=hidden */
#  define EXPORT __attribute__((__visibility__("default")))
# endif
#else
# if defined (_MSC_VER)
#  define EXPORT __declspec(dllimport)
# else
#  define EXPORT
# endif
#endif

#include <jansson_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* version */

#define JANSSON_MAJOR_VERSION  2
#define JANSSON_MINOR_VERSION  2
#define JANSSON_MICRO_VERSION  1

/* Micro version is omitted if it's 0 */
#define JANSSON_VERSION  "2.2.1"

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
   for numeric comparisons, e.g. #if JANSSON_VERSION_HEX >= ... */
#define JANSSON_VERSION_HEX  ((JANSSON_MAJOR_VERSION << 16) |   \
                              (JANSSON_MINOR_VERSION << 8)  |   \
                              (JANSSON_MICRO_VERSION << 0))


/* types */

typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_REAL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL
} json_type;

typedef struct {
    json_type type;
    size_t refcount;
} json_t;

#if JSON_INTEGER_IS_LONG_LONG
#ifdef _WIN32
#define JSON_INTEGER_FORMAT "I64d"
#else
#define JSON_INTEGER_FORMAT "lld"
#endif
typedef long long json_int_t;
#else
#define JSON_INTEGER_FORMAT "ld"
typedef long json_int_t;
#endif /* JSON_INTEGER_IS_LONG_LONG */

#define json_typeof(json)      ((json)->type)
#define json_is_object(json)   (json && json_typeof(json) == JSON_OBJECT)
#define json_is_array(json)    (json && json_typeof(json) == JSON_ARRAY)
#define json_is_string(json)   (json && json_typeof(json) == JSON_STRING)
#define json_is_integer(json)  (json && json_typeof(json) == JSON_INTEGER)
#define json_is_real(json)     (json && json_typeof(json) == JSON_REAL)
#define json_is_number(json)   (json_is_integer(json) || json_is_real(json))
#define json_is_true(json)     (json && json_typeof(json) == JSON_TRUE)
#define json_is_false(json)    (json && json_typeof(json) == JSON_FALSE)
#define json_is_boolean(json)  (json_is_true(json) || json_is_false(json))
#define json_is_null(json)     (json && json_typeof(json) == JSON_NULL)

/* construction, destruction, reference counting */

EXPORT json_t *json_object(void);
EXPORT json_t *json_array(void);
EXPORT json_t *json_string(const char *value);
EXPORT json_t *json_string_nocheck(const char *value);
EXPORT json_t *json_integer(json_int_t value);
EXPORT json_t *json_real(double value);
EXPORT json_t *json_true(void);
EXPORT json_t *json_false(void);
EXPORT json_t *json_null(void);

static JSON_INLINE
json_t *json_incref(json_t *json)
{
    if(json && json->refcount != (size_t)-1)
        ++json->refcount;
    return json;
}

/* do not call json_delete directly */
EXPORT void json_delete(json_t *json);

static JSON_INLINE
void json_decref(json_t *json)
{
    if(json && json->refcount != (size_t)-1 && --json->refcount == 0)
        json_delete(json);
}


/* error reporting */

#define JSON_ERROR_TEXT_LENGTH    160
#define JSON_ERROR_SOURCE_LENGTH   80

typedef struct {
    int line;
    int column;
    int position;
    char source[JSON_ERROR_SOURCE_LENGTH];
    char text[JSON_ERROR_TEXT_LENGTH];
} json_error_t;


/* getters, setters, manipulation */

EXPORT size_t json_object_size(const json_t *object);
EXPORT json_t *json_object_get(const json_t *object, const char *key);
EXPORT int json_object_set_new(json_t *object, const char *key, json_t *value);
EXPORT int json_object_set_new_nocheck(json_t *object, const char *key, json_t *value);
EXPORT int json_object_del(json_t *object, const char *key);
EXPORT int json_object_clear(json_t *object);
EXPORT int json_object_update(json_t *object, json_t *other);
EXPORT void *json_object_iter(json_t *object);
EXPORT void *json_object_iter_at(json_t *object, const char *key);
EXPORT void *json_object_iter_next(json_t *object, void *iter);
EXPORT const char *json_object_iter_key(void *iter);
EXPORT json_t *json_object_iter_value(void *iter);
EXPORT int json_object_iter_set_new(json_t *object, void *iter, json_t *value);

static JSON_INLINE
int json_object_set(json_t *object, const char *key, json_t *value)
{
    return json_object_set_new(object, key, json_incref(value));
}

static JSON_INLINE
int json_object_set_nocheck(json_t *object, const char *key, json_t *value)
{
    return json_object_set_new_nocheck(object, key, json_incref(value));
}

static JSON_INLINE
int json_object_iter_set(json_t *object, void *iter, json_t *value)
{
    return json_object_iter_set_new(object, iter, json_incref(value));
}

EXPORT size_t json_array_size(const json_t *array);
EXPORT json_t *json_array_get(const json_t *array, size_t index);
EXPORT int json_array_set_new(json_t *array, size_t index, json_t *value);
EXPORT int json_array_append_new(json_t *array, json_t *value);
EXPORT int json_array_insert_new(json_t *array, size_t index, json_t *value);
EXPORT int json_array_remove(json_t *array, size_t index);
EXPORT int json_array_clear(json_t *array);
EXPORT int json_array_extend(json_t *array, json_t *other);

static JSON_INLINE
int json_array_set(json_t *array, size_t index, json_t *value)
{
    return json_array_set_new(array, index, json_incref(value));
}

static JSON_INLINE
int json_array_append(json_t *array, json_t *value)
{
    return json_array_append_new(array, json_incref(value));
}

static JSON_INLINE
int json_array_insert(json_t *array, size_t index, json_t *value)
{
    return json_array_insert_new(array, index, json_incref(value));
}

EXPORT const char *json_string_value(const json_t *string);
EXPORT json_int_t json_integer_value(const json_t *integer);
EXPORT double json_real_value(const json_t *real);
EXPORT double json_number_value(const json_t *json);

EXPORT int json_string_set(json_t *string, const char *value);
EXPORT int json_string_set_nocheck(json_t *string, const char *value);
EXPORT int json_integer_set(json_t *integer, json_int_t value);
EXPORT int json_real_set(json_t *real, double value);


/* pack, unpack */

EXPORT json_t *json_pack(const char *fmt, ...);
EXPORT json_t *json_pack_ex(json_error_t *error, size_t flags, const char *fmt, ...);
EXPORT json_t *json_vpack_ex(json_error_t *error, size_t flags, const char *fmt, va_list ap);

#define JSON_VALIDATE_ONLY  0x1
#define JSON_STRICT         0x2

EXPORT int json_unpack(json_t *root, const char *fmt, ...);
EXPORT int json_unpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...);
EXPORT int json_vunpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap);


/* equality */

EXPORT int json_equal(json_t *value1, json_t *value2);


/* copying */

EXPORT json_t *json_copy(json_t *value);
EXPORT json_t *json_deep_copy(json_t *value);


/* decoding */

#define JSON_REJECT_DUPLICATES 0x1
#define JSON_DISABLE_EOF_CHECK 0x2
#define JSON_DECODE_ANY        0x4

EXPORT json_t *json_loads(const char *input, size_t flags, json_error_t *error);
EXPORT json_t *json_loadb(const char *buffer, size_t buflen, size_t flags, json_error_t *error);
EXPORT json_t *json_loadf(FILE *input, size_t flags, json_error_t *error);
EXPORT json_t *json_load_file(const char *path, size_t flags, json_error_t *error);

/* encoding */

#define JSON_INDENT(n)      (n & 0x1F)
#define JSON_COMPACT        0x20
#define JSON_ENSURE_ASCII   0x40
#define JSON_SORT_KEYS      0x80
#define JSON_PRESERVE_ORDER 0x100
#define JSON_ENCODE_ANY     0x200

typedef int (*json_dump_callback_t)(const char *buffer, size_t size, void *data);

EXPORT char *json_dumps(const json_t *json, size_t flags);
EXPORT int json_dumpf(const json_t *json, FILE *output, size_t flags);
EXPORT int json_dump_file(const json_t *json, const char *path, size_t flags);
EXPORT int json_dump_callback(const json_t *json, json_dump_callback_t callback, void *data, size_t flags);

/* custom memory allocation */

typedef void *(*json_malloc_t)(size_t);
typedef void (*json_free_t)(void *);

EXPORT void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn);

#ifdef __cplusplus
}
#endif

#undef EXPORT

#endif
