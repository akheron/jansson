/*
 * Copyright (c) 2009-2011 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <jansson.h>

#define failhdr fprintf(stderr, "%s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__)

#define fail(msg)                                                \
    do {                                                         \
        failhdr;                                                 \
        fprintf(stderr, "%s\n", msg);                            \
        exit(1);                                                 \
    } while(0)

/* Assumes json_error_t error */
#define check_error(text_, source_, line_, column_, position_)          \
    do {                                                                \
        if(strcmp(error.text, text_) != 0) {                            \
            failhdr;                                                    \
            fprintf(stderr, "text: \"%s\" != \"%s\"\n", error.text, text_); \
            exit(1);                                                    \
        }                                                               \
        if(strcmp(error.source, source_) != 0) {                        \
            failhdr;                                                    \
                                                                        \
            fprintf(stderr, "source: \"%s\" != \"%s\"\n", error.source, source_); \
            exit(1);                                                    \
        }                                                               \
        if(error.line != line_) {                                       \
            failhdr;                                                    \
            fprintf(stderr, "line: %d != %d\n", error.line, line_);     \
            exit(1);                                                    \
        }                                                               \
        if(error.column != column_) {                                   \
            failhdr;                                                    \
            fprintf(stderr, "column: %d != %d\n", error.column, column_); \
            exit(1);                                                    \
        }                                                               \
        if(error.position != position_) {                               \
            failhdr;                                                    \
            fprintf(stderr, "position: %d != %d\n", error.position, position_); \
            exit(1);                                                    \
        }                                                               \
    } while(0)


static void run_tests(void);

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    run_tests();
    return 0;
}

#ifdef _MSC_VER
typedef long ssize_t;
#endif

#endif
