/*
 * Copyright (c) 2009, 2010 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef TESTPROGS_UTIL_H
#define TESTPROGS_UTIL_H

#include <stdlib.h>

#define fail(msg)                                                \
    do {                                                         \
        fprintf(stderr, "%s:%s:%d: %s\n",                        \
                __FILE__, __FUNCTION__, __LINE__, msg);          \
        exit(1);                                                 \
    } while(0)

#endif
