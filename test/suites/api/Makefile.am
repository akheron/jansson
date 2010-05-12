EXTRA_DIST = run

check_PROGRAMS = \
	test_array \
	test_equal \
	test_copy \
	test_dump \
	test_load \
	test_simple \
	test_number \
	test_object

test_array_SOURCES = test_array.c util.h
test_copy_SOURCES = test_copy.c util.h
test_dump_SOURCES = test_dump.c util.h
test_load_SOURCES = test_load.c util.h
test_simple_SOURCES = test_simple.c util.h
test_number_SOURCES = test_number.c util.h
test_object_SOURCES = test_object.c util.h

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Werror
LDFLAGS = -static  # for speed and Valgrind
LDADD = $(top_builddir)/src/libjansson.la
