check_PROGRAMS = test_array test_load test_simple test_number test_object

test_array_SOURCES = test_array.c util.h
test_load_SOURCES = test_load.c util.h
test_simple_SOURCES = test_simple.c util.h
test_number_SOURCES = test_number.c util.h
test_object_SOURCES = test_object.c util.h

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Werror
LDFLAGS = -static  # for speed and Valgrind
LDADD = ../../src/libjansson.la
