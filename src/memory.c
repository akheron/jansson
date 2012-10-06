/*
 * Copyright (c) 2009-2012 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2011-2012 Basile Starynkevitch <basile@starynkevitch.net>
 *
 * Jansson is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>

#include "jansson.h"
#include "jansson_private.h"

/* memory function pointers */
static json_malloc_t do_malloc = malloc;
static json_free_t do_free = free;

/* current context */
static json_context_t* jsonp_current_context_ptr = NULL;
static json_context_t jsonp_current_context;

static void jsonp_overwrite_memset(void *ptr, size_t size)
{
    if(size==0 || ptr==NULL)
        return;
    memset( ptr, 0, size);
}

json_context_t *jsonp_context(void)
{
    if(jsonp_current_context_ptr)
        return jsonp_current_context_ptr;

    memset(&jsonp_current_context, 0, sizeof(json_context_t));

    jsonp_current_context.have_bigint = 0;
    jsonp_current_context.have_bigreal = 0;

    jsonp_current_context_ptr = &jsonp_current_context;
    return jsonp_current_context_ptr;
}

/* memory functions */
void jsonp_overwrite(void *ptr, size_t size)
{
    jsonp_overwrite_memset(ptr, size);
}

void *jsonp_malloc(size_t size)
{
    if(!size)
        return NULL;

    return (*do_malloc)(size);
}

void jsonp_free(void *ptr)
{
    if(!ptr)
        return;

    (*do_free)(ptr);
}

char *jsonp_strdup(const char *str)
{
    char *new_str;
    size_t size = strlen(str);

    new_str = jsonp_malloc(size + 1);
    if(!new_str)
        return NULL;

    memcpy(new_str, str, size);
    new_str[size] = '\0';
    return new_str;
}

void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn)
{
    if( malloc_fn == NULL )
        malloc_fn = malloc;
    if( free_fn == NULL )
        free_fn = free;

    do_malloc = malloc_fn;
    do_free = free_fn;
}

void json_set_biginteger_funcs(const json_bigint_funcs_t* functions)
{
    json_context_t *ctx = jsonp_context();
    if(!functions) {
        ctx->have_bigint = 0;
        memset(&ctx->bigint, 0, sizeof(json_bigint_funcs_t));
    }
    else {
        ctx->have_bigint = 1;
        memcpy(&ctx->bigint, functions, sizeof(json_bigint_funcs_t));
    }
}

void json_set_bigreal_funcs(const json_bigreal_funcs_t* functions)
{
    json_context_t *ctx = jsonp_context();
    if(!functions) {
        ctx->have_bigreal = 0;
        memset(&ctx->bigreal, 0, sizeof(json_bigreal_funcs_t));
    }
    else {
        ctx->have_bigreal = 1;
        memcpy(&ctx->bigreal, functions, sizeof(json_bigreal_funcs_t));
    }
}
