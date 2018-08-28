/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <math.h>
#include <jansson.h>
#include "util.h"

#ifdef INFINITY
// This test triggers "warning C4756: overflow in constant arithmetic"
// in Visual Studio. This warning is triggered here by design, so disable it.
// (This can only be done on function level so we keep these tests separate)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4756)
#endif
static void test_inifity()
{
#if JSON_HAVE_FLOAT
    json_t *real = json_real(INFINITY);
    if (real != NULL)
       fail("could construct a real from Inf");

    real = json_real(1.0);
    if (json_real_set(real, INFINITY) != -1)
	    fail("could set a real to Inf");

    if (json_real_value(real) != 1.0)
       fail("real value changed unexpectedly");

    json_decref(real);
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}
#endif // INFINITY

static void run_tests()
{
    json_t *integer;
#if JSON_HAVE_FLOAT
    json_t *real;
    double d;
#endif
    json_int_t i;

    integer = json_integer(5);
#if JSON_HAVE_FLOAT
    real = json_real(100.1);
#endif

    if(!integer)
        fail("unable to create integer");
#if JSON_HAVE_FLOAT
    if(!real)
        fail("unable to create real");
#endif

    i = json_integer_value(integer);
    if(i != 5)
        fail("wrong integer value");

#if JSON_HAVE_FLOAT
    d = json_real_value(real);
    if(d != 100.1)
        fail("wrong real value");

    d = json_number_value(integer);
    if(d != 5.0)
        fail("wrong number value");
    d = json_number_value(real);
    if(d != 100.1)
        fail("wrong number value");

    json_decref(real);
#endif

    json_decref(integer);

#if JSON_HAVE_FLOAT
#ifdef NAN
    real = json_real(NAN);
    if(real != NULL)
        fail("could construct a real from NaN");

    real = json_real(1.0);
    if(json_real_set(real, NAN) != -1)
        fail("could set a real to NaN");

    if(json_real_value(real) != 1.0)
        fail("real value changed unexpectedly");

    json_decref(real);
#endif
#endif

#ifdef INFINITY
    test_inifity();
#endif
}
