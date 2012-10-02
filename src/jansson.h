/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_H
#define JANSSON_H

#include <stdio.h>
#include <stdlib.h>  /* for size_t */
#include <stdarg.h>

#include <jansson_config.h>
#if USE_GNU_MP
#include <gmp.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* version */

#define JANSSON_MAJOR_VERSION  2
#define JANSSON_MINOR_VERSION  4
#define JANSSON_MICRO_VERSION  99

/* Micro version is omitted if it's 0 */
#define JANSSON_VERSION  "2.5-dev"

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
   for numeric comparisons, e.g. #if JANSSON_VERSION_HEX >= ... */
#define JANSSON_VERSION_HEX  ((JANSSON_MAJOR_VERSION << 16) |   \
                              (JANSSON_MINOR_VERSION << 8)  |   \
                              (JANSSON_MICRO_VERSION << 0))


/* types */

struct json_context; /* forward reference */

typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_REAL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL,
    JSON_BIGINTEGER,
    JSON_BIGREAL
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

#ifndef JSON_BIGZ_TYPE
#define JSON_BIGZ_TYPE void
#endif
#ifndef JSON_BIGR_TYPE
#define JSON_BIGR_TYPE void
#endif

typedef JSON_BIGZ_TYPE * json_bigz_t;
typedef JSON_BIGR_TYPE * json_bigr_t;
typedef JSON_BIGZ_TYPE const * json_bigz_const_t;
typedef JSON_BIGR_TYPE const * json_bigr_const_t;


#define json_typeof(json)      ((json)->type)
#define json_is_object(json)   (json && json_typeof(json) == JSON_OBJECT)
#define json_is_array(json)    (json && json_typeof(json) == JSON_ARRAY)
#define json_is_string(json)   (json && json_typeof(json) == JSON_STRING)
#define json_is_integer(json)  (json && json_typeof(json) == JSON_INTEGER)
#define json_is_biginteger(json) (json && json_typeof(json) == JSON_BIGINTEGER)
#define json_is_anyinteger(json) (json_is_integer(json) || json_is_biginteger(json))
#define json_is_real(json)     (json && json_typeof(json) == JSON_REAL)
#define json_is_bigreal(json)  (json && json_typeof(json) == JSON_BIGREAL)
#define json_is_anyreal(json)  (json_is_real(json) || json_is_bigreal(json))
#define json_is_number(json)   (json_is_integer(json) || json_is_real(json))
#define json_is_bignumber(json) (json_is_biginteger(json) || json_is_bigreal(json))
#define json_is_anynumber(json) (json_is_number(json) || json_is_bignumber(json))
#define json_is_true(json)     (json && json_typeof(json) == JSON_TRUE)
#define json_is_false(json)    (json && json_typeof(json) == JSON_FALSE)
#define json_is_boolean(json)  (json_is_true(json) || json_is_false(json))
#define json_is_null(json)     (json && json_typeof(json) == JSON_NULL)

/* construction, destruction, reference counting */

json_t *json_object(void);
json_t *json_array(void);
json_t *json_string(const char *value);
json_t *json_string_nocheck(const char *value);
json_t *json_integer(json_int_t value);
json_t *json_real(double value);
json_t *json_true(void);
json_t *json_false(void);
#define json_boolean(val)      ((val) ? json_true() : json_false())
json_t *json_null(void);
json_t *json_biginteger(json_bigz_const_t value);
json_t *json_bigreal(json_bigr_const_t value);

static JSON_INLINE
json_t *json_incref(json_t *json)
{
    if(json && json->refcount != (size_t)-1)
        ++json->refcount;
    return json;
}

/* do not call json_delete directly */
void json_delete(json_t *json);

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

size_t json_object_size(const json_t *object);
json_t *json_object_get(const json_t *object, const char *key);
int json_object_set_new(json_t *object, const char *key, json_t *value);
int json_object_set_new_nocheck(json_t *object, const char *key, json_t *value);
int json_object_del(json_t *object, const char *key);
int json_object_clear(json_t *object);
int json_object_update(json_t *object, json_t *other);
int json_object_update_existing(json_t *object, json_t *other);
int json_object_update_missing(json_t *object, json_t *other);
void *json_object_iter(json_t *object);
void *json_object_iter_at(json_t *object, const char *key);
void *json_object_key_to_iter(const char *key);
void *json_object_iter_next(json_t *object, void *iter);
const char *json_object_iter_key(void *iter);
json_t *json_object_iter_value(void *iter);
int json_object_iter_set_new(json_t *object, void *iter, json_t *value);

#define json_object_foreach(object, key, value) \
    for(key = json_object_iter_key(json_object_iter(object)); \
        key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
        key = json_object_iter_key(json_object_iter_next(object, json_object_key_to_iter(key))))

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

size_t json_array_size(const json_t *array);
json_t *json_array_get(const json_t *array, size_t index);
int json_array_set_new(json_t *array, size_t index, json_t *value);
int json_array_append_new(json_t *array, json_t *value);
int json_array_insert_new(json_t *array, size_t index, json_t *value);
int json_array_remove(json_t *array, size_t index);
int json_array_clear(json_t *array);
int json_array_extend(json_t *array, json_t *other);

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

const char *json_string_value(const json_t *string);
json_int_t json_integer_value(const json_t *integer);
double json_real_value(const json_t *real);
double json_number_value(const json_t *json);

int json_string_set(json_t *string, const char *value);
int json_string_set_nocheck(json_t *string, const char *value);
int json_integer_set(json_t *integer, json_int_t value);
int json_real_set(json_t *real, double value);

json_bigz_const_t json_biginteger_value(const json_t *biginteger);
int json_biginteger_set(json_t *biginteger, json_bigz_const_t mpz);

json_bigr_const_t json_bigreal_value(const json_t *bigreal);
int json_bigreal_set(json_t *bigreal, json_bigr_const_t mpf);


/* pack, unpack */

json_t *json_pack(const char *fmt, ...);
json_t *json_pack_ex(json_error_t *error, size_t flags, const char *fmt, ...);
json_t *json_vpack_ex(json_error_t *error, size_t flags, const char *fmt, va_list ap);

#define JSON_VALIDATE_ONLY  0x1
#define JSON_STRICT         0x2

int json_unpack(json_t *root, const char *fmt, ...);
int json_unpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...);
int json_vunpack_ex(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap);


/* equality */

int json_equal(json_t *value1, json_t *value2);


/* copying */

json_t *json_copy(json_t *value);
json_t *json_deep_copy(json_t *value);


/* decoding */

#define JSON_REJECT_DUPLICATES 0x1
#define JSON_DISABLE_EOF_CHECK 0x2
#define JSON_DECODE_ANY        0x4

typedef size_t (*json_load_callback_t)(void *buffer, size_t buflen, void *data);

json_t *json_loads(const char *input, size_t flags, json_error_t *error);
json_t *json_loadb(const char *buffer, size_t buflen, size_t flags, json_error_t *error);
json_t *json_loadf(FILE *input, size_t flags, json_error_t *error);
json_t *json_load_file(const char *path, size_t flags, json_error_t *error);
json_t *json_load_callback(json_load_callback_t callback, void *data, size_t flags, json_error_t *error);


/* encoding */

#define JSON_INDENT(n)      (n & 0x1F)
#define JSON_COMPACT        0x20
#define JSON_ENSURE_ASCII   0x40
#define JSON_SORT_KEYS      0x80
#define JSON_PRESERVE_ORDER 0x100
#define JSON_ENCODE_ANY     0x200
#define JSON_ESCAPE_SLASH   0x400
#define JSON_USE_BIGINT         0x800
#define JSON_USE_BIGINT_ALWAYS  0x1000
#define JSON_USE_BIGREAL        0x2000
#define JSON_USE_BIGREAL_ALWAYS 0x4000

typedef int (*json_dump_callback_t)(const char *buffer, size_t size, void *data);

char *json_dumps(const json_t *json, size_t flags);
int json_dumpf(const json_t *json, FILE *output, size_t flags);
int json_dump_file(const json_t *json, const char *path, size_t flags);
int json_dump_callback(const json_t *json, json_dump_callback_t callback, void *data, size_t flags);

/* custom memory allocation */

typedef void *(*json_malloc_t)(size_t);
typedef void (*json_free_t)(void *);

void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn);

/* big integers */

typedef json_bigz_t (*json_bigint_copy_t)(json_bigz_const_t bignum);
typedef void (*json_bigint_del_t)(json_bigz_t bignum);
typedef int (*json_bigint_cmp_t)(json_bigz_const_t bignum1,
                                     json_bigz_const_t bignum2);
typedef int (*json_bigint_to_str_t)(json_bigz_const_t bignum,
                                     char *buffer, size_t size);
typedef json_bigz_t (*json_bigint_from_str_t)(const char *value);
typedef json_bigz_t (*json_bigint_from_int_t)(json_int_t value);

typedef struct {
    json_bigint_copy_t copy_fn;
    json_bigint_del_t delete_fn;
    json_bigint_cmp_t compare_fn;
    json_bigint_to_str_t to_string_fn;
    json_bigint_from_str_t from_string_fn;
    json_bigint_from_int_t from_int_fn;
} json_bigint_funcs_t;

void json_set_biginteger_funcs(const json_bigint_funcs_t* functions);


/* big reals */

typedef json_bigr_t (*json_bigreal_copy_t)(json_bigr_const_t bigreal);
typedef void (*json_bigreal_del_t)(json_bigr_t bigreal);
typedef int (*json_bigreal_cmp_t)(json_bigr_const_t bigreal1,
                 json_bigr_const_t bigreal2);
typedef int (*json_bigreal_to_str_t)(json_bigr_const_t bigreal,
                 char *buffer, size_t size);
typedef json_bigr_t (*json_bigreal_from_str_t)(const char *value);
typedef json_bigr_t (*json_bigreal_from_real_t)(double value);

typedef struct {
    json_bigreal_copy_t copy_fn;
    json_bigreal_del_t delete_fn;
    json_bigreal_cmp_t compare_fn;
    json_bigreal_to_str_t to_string_fn;
    json_bigreal_from_str_t from_string_fn;
    json_bigreal_from_real_t from_real_fn;
} json_bigreal_funcs_t;

void json_set_bigreal_funcs(const json_bigreal_funcs_t* functions);



#ifdef __cplusplus
}
#endif

#endif