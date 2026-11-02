/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "util.h"
#include <jansson.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef INFINITY
// This test triggers "warning C4756: overflow in constant arithmetic"
// in Visual Studio. This warning is triggered here by design, so disable it.
// (This can only be done on function level so we keep these tests separate)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4756)
#endif
static void test_inifity() {
    json_t *real = json_real(INFINITY);
    if (real != NULL)
        fail("could construct a real from Inf");

    real = json_real(1.0);
    if (json_real_set(real, INFINITY) != -1)
        fail("could set a real to Inf");

    if (json_real_value(real) != 1.0)
        fail("real value changed unexpectedly");

    json_decref(real);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}
#endif // INFINITY

static void test_bad_args(void) {
    json_t *txt = json_string("test");

    if (json_integer_value(NULL) != 0)
        fail("json_integer_value did not return 0 for non-integer");
    if (json_integer_value(txt) != 0)
        fail("json_integer_value did not return 0 for non-integer");

    if (!json_integer_set(NULL, 0))
        fail("json_integer_set did not return error for non-integer");
    if (!json_integer_set(txt, 0))
        fail("json_integer_set did not return error for non-integer");

    if (json_real_value(NULL) != 0.0)
        fail("json_real_value did not return 0.0 for non-real");
    if (json_real_value(txt) != 0.0)
        fail("json_real_value did not return 0.0 for non-real");

    if (!json_real_set(NULL, 0.0))
        fail("json_real_set did not return error for non-real");
    if (!json_real_set(txt, 0.0))
        fail("json_real_set did not return error for non-real");

    if (json_number_value(NULL) != 0.0)
        fail("json_number_value did not return 0.0 for non-numeric");
    if (json_number_value(txt) != 0.0)
        fail("json_number_value did not return 0.0 for non-numeric");

    if (txt->refcount != 1)
        fail("unexpected reference count for txt");

    json_decref(txt);
}

/* CMake builds of the tests have no config header; ISO C has setlocale(). */
#if !defined(HAVE_CONFIG_H) || (HAVE_LOCALE_H && defined(HAVE_SETLOCALE))
#include <locale.h>

/* ps_AF.UTF-8 writes the decimal point as U+066B, 2 bytes in UTF-8. Jansson
   swaps '.' for the locale's point before strtod and back after printing. */
static int set_multibyte_point_locale(void) {
    char buf[32];

    if (!setlocale(LC_NUMERIC, "ps_AF.UTF-8") && !setlocale(LC_NUMERIC, "ps_AF.utf8"))
        return 0;
    /* sprintf: the API tests also build with MSVC before 2015, which has no snprintf. */
    return sprintf(buf, "%#.0f", 1.0) > 2;
}

static void test_multibyte_decimal_point(void) {
    static const char *const reals[] = {
        "1.5", "-2.25e3", "0.125", "1e-7", "12345678901234.5",
        "0.1234567890123456789012345678901234567890123",
    };
    double expected[sizeof(reals) / sizeof(reals[0])];
    char saved[64], *current;
    size_t i;

    current = setlocale(LC_NUMERIC, NULL);
    if (!current || strlen(current) >= sizeof(saved))
        return;
    strcpy(saved, current);

    setlocale(LC_NUMERIC, "C");
    for (i = 0; i < sizeof(reals) / sizeof(reals[0]); i++)
        expected[i] = strtod(reals[i], NULL);

    if (!set_multibyte_point_locale()) {
        fprintf(stderr, "test_multibyte_decimal_point: skipped, no ps_AF.UTF-8 locale\n");
        setlocale(LC_NUMERIC, saved);
        return;
    }

    for (i = 0; i < sizeof(reals) / sizeof(reals[0]); i++) {
        char doc[64], *dumped;
        json_error_t error;
        json_t *array, *value, *reloaded;

        sprintf(doc, "[%s]", reals[i]); /* every real is under 60 bytes */
        array = json_loads(doc, 0, &error);
        value = json_array_get(array, 0);
        if (!json_is_real(value) || json_real_value(value) != expected[i])
            fail("a real did not load under a multibyte decimal point");

        /* The dump must be plain JSON: '.' and no byte of the locale's point.
           Reloading it in the C locale must give the same value back. */
        dumped = json_dumps(array, JSON_COMPACT);
        if (!dumped || strpbrk(dumped, "\xd9\xab"))
            fail("a real dumped with the locale's decimal point");
        setlocale(LC_NUMERIC, "C");
        reloaded = json_loads(dumped, 0, &error);
        if (json_real_value(json_array_get(reloaded, 0)) != expected[i])
            fail("a real dumped under a multibyte decimal point did not reload");
        set_multibyte_point_locale();

        json_decref(reloaded);
        free(dumped);
        json_decref(array);
    }
    setlocale(LC_NUMERIC, saved);
}
#endif

static void run_tests() {
    json_t *integer, *real;
    json_int_t i;
    double d;

    integer = json_integer(5);
    real = json_real(100.1);

    if (!integer)
        fail("unable to create integer");
    if (!real)
        fail("unable to create real");

    i = json_integer_value(integer);
    if (i != 5)
        fail("wrong integer value");

    d = json_real_value(real);
    if (d != 100.1)
        fail("wrong real value");

    d = json_number_value(integer);
    if (d != 5.0)
        fail("wrong number value");
    d = json_number_value(real);
    if (d != 100.1)
        fail("wrong number value");

    json_decref(integer);
    json_decref(real);

#ifdef NAN
    real = json_real(NAN);
    if (real != NULL)
        fail("could construct a real from NaN");

    real = json_real(1.0);
    if (json_real_set(real, NAN) != -1)
        fail("could set a real to NaN");

    if (json_real_value(real) != 1.0)
        fail("real value changed unexpectedly");

    json_decref(real);
#endif

#ifdef INFINITY
    test_inifity();
#endif
    test_bad_args();
#if !defined(HAVE_CONFIG_H) || (HAVE_LOCALE_H && defined(HAVE_SETLOCALE))
    test_multibyte_decimal_point();
#endif
}
