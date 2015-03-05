/*
 * Copyright (c) 2009-2014 Petri Lehtinen <petri@digip.org>
 * Copyright (c) 2011-2012 Basile Starynkevitch <basile@starynkevitch.net>
 *
 * Jansson is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>

#include "jansson.h"
#include "jansson_private.h"

/* C89 allows these to be macros */
#undef malloc
#undef free

/* memory function pointers */
static union { 
  json_malloc_t     normal;
  json_malloc_arg_t with_arg;
} do_malloc = {.normal = malloc};

static union {
  json_free_t     normal;
  json_free_arg_t with_arg;
} do_free = {.normal = free};

/* Argument to pass to alloc functions which accept user argument. If NULL, normal alloc functions are used. */
static void *user_arg = NULL;


void *jsonp_malloc(size_t size)
{
    if(!size)
        return NULL;
    
    return (user_arg == NULL) ? (do_malloc.normal)(size) : (do_malloc.with_arg)(size, user_arg);
}

void jsonp_free(void *ptr)
{
    if(!ptr)
        return;

    if (user_arg == NULL) {
        (do_free.normal)(ptr);
    } else {
        (do_free.with_arg)(ptr, user_arg);
    }
}

char *jsonp_strdup(const char *str)
{
    return jsonp_strndup(str, strlen(str));
}

char *jsonp_strndup(const char *str, size_t len)
{
    char *new_str;

    new_str = jsonp_malloc(len + 1);
    if(!new_str)
        return NULL;

    memcpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn)
{
    do_malloc.normal = malloc_fn;
    do_free.normal = free_fn;
    user_arg = NULL; 
}

void json_set_alloc_funcs_arg(json_malloc_arg_t malloc_fn, json_free_arg_t free_fn, void *arg)
{
    if (arg == NULL) {
        json_set_alloc_funcs((json_malloc_t) malloc_fn, (json_free_t) free_fn);
    } else {
      do_malloc.with_arg = malloc_fn;
      do_free.with_arg = free_fn;
      user_arg = arg;
    }
}

