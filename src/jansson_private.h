/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_PRIVATE_H
#define JANSSON_PRIVATE_H

#include <stddef.h>
#include "jansson.h"
#include "hashtable.h"
#include "strbuffer.h"

#define container_of(ptr_, type_, member_)  \
    ((type_ *)((char *)ptr_ - offsetof(type_, member_)))

/* On some platforms, max() may already be defined */
#ifndef max
#define max(a, b)  ((a) > (b) ? (a) : (b))
#endif

/* va_copy is a C99 feature. In C89 implementations, it's sometimes
   available as __va_copy. If not, memcpy() should do the trick. */
#ifndef va_copy
#ifdef __va_copy
#define va_copy __va_copy
#else
#define va_copy(a, b)  memcpy(&(a), &(b), sizeof(va_list))
#endif
#endif

typedef struct {
    json_t json;
    hashtable_t hashtable;
    size_t serial;
    int visited;
} json_object_t;

typedef struct {
    json_t json;
    size_t size;
    size_t entries;
    json_t **table;
    int visited;
} json_array_t;

typedef struct {
    json_t json;
    char *value;
} json_string_t;

typedef struct {
    json_t json;
    double value;
} json_real_t;

typedef struct {
    json_t json;
    json_bigr_t *value;
} json_bigreal_t;

typedef struct {
    json_t json;
    json_int_t value;
} json_integer_t;

typedef struct {
    json_t json;
    json_bigz_t *value;
} json_biginteger_t;


#define json_to_object(json_)  container_of(json_, json_object_t, json)
#define json_to_array(json_)   container_of(json_, json_array_t, json)
#define json_to_string(json_)  container_of(json_, json_string_t, json)
#define json_to_real(json_)   container_of(json_, json_real_t, json)
#define json_to_integer(json_) container_of(json_, json_integer_t, json)
#define json_to_biginteger(json_) container_of(json_, json_biginteger_t, json)
#define json_to_bigreal(json_) container_of(json_, json_bigreal_t, json)

void jsonp_error_init(json_error_t *error, const char *source);
void jsonp_error_set_source(json_error_t *error, const char *source);
void jsonp_error_set(json_error_t *error, int line, int column,
                     size_t position, const char *msg, ...);
void jsonp_error_vset(json_error_t *error, int line, int column,
                      size_t position, const char *msg, va_list ap);

/* Locale independent string<->double conversions */
int jsonp_strtod(strbuffer_t *strbuffer, double *out);
int jsonp_dtostr(char *buffer, size_t size, double value);

/* For estimating precision needed to store a real number */
int jsonp_count_significand_digits(strbuffer_t *strbuffer);

/* Global context */

typedef struct json_context {
    int have_bigint;
    int have_bigreal;
    json_bigint_funcs_t bigint;
    json_bigreal_funcs_t bigreal;
} json_context_t;

json_context_t *jsonp_context(void);

/* Wrappers for custom memory functions */
void *jsonp_malloc(size_t size);
void jsonp_free(void *ptr);
void jsonp_overwrite(void *ptr, size_t size);
char *jsonp_strdup(const char *str);

/* Windows compatibility */
#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

#endif
