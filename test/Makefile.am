check_PROGRAMS = load_dump loadf_dumpf loads_dumps

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Werror
LDFLAGS = -static  # for speed and Valgrind
LDADD = ../src/libjansson.la

TESTS = test-invalid test-valid

clean-local:
	rm -rf testlogs
