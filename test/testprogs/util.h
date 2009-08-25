/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef TESTPROGS_UTIL_H
#define TESTPROGS_UTIL_H

#define fail(msg)                                                \
    do {                                                         \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1;                                                \
    } while(0)

#endif
