/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
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
#undef realloc
#undef free

/* memory function pointers */
static json_malloc_t do_malloc = malloc;
static json_realloc_t do_realloc = realloc;
static json_free_t do_free = free;

void *jsonp_malloc(size_t size) {
    if (!size)
        return NULL;

    return (*do_malloc)(size);
}

void jsonp_free(void *ptr) {
    if (!ptr)
        return;

    (*do_free)(ptr);
}

void *jsonp_realloc(void *ptr, size_t originalSize, size_t newSize) {
    void *newMemory;

    if (do_realloc)
        return (*do_realloc)(ptr, newSize);

    // realloc emulation using malloc and free
    if (newSize == 0) {
        if (ptr != NULL)
            (*do_free)(ptr);

        return NULL;
    } else {
        newMemory = (*do_malloc)(newSize);

        if ((newMemory != NULL) && (ptr != NULL)) {
            memcpy(newMemory, ptr, (originalSize < newSize) ? originalSize : newSize);

            (*do_free)(ptr);
        }

        return newMemory;
    }
}

char *jsonp_strndup(const char *str, size_t len) {
    char *new_str;

    new_str = jsonp_malloc(len + 1);
    if (!new_str)
        return NULL;

    memcpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn) {
    do_malloc = malloc_fn;
    do_realloc = NULL;
    do_free = free_fn;
}

void json_set_alloc_funcs2(json_malloc_t malloc_fn, json_realloc_t realloc_fn,
                           json_free_t free_fn) {
    do_malloc = malloc_fn;
    do_realloc = realloc_fn;
    do_free = free_fn;
}

void json_get_alloc_funcs(json_malloc_t *malloc_fn, json_free_t *free_fn) {
    if (malloc_fn)
        *malloc_fn = do_malloc;
    if (free_fn)
        *free_fn = do_free;
}
void json_get_alloc_funcs2(json_malloc_t *malloc_fn, json_realloc_t *realloc_fn,
                           json_free_t *free_fn) {
    if (malloc_fn)
        *malloc_fn = do_malloc;
    if (realloc_fn)
        *realloc_fn = do_realloc;
    if (free_fn)
        *free_fn = do_free;
}
