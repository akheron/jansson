check_PROGRAMS = loadf_dumpf loads_dumps load_file_dump_file

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Werror
LDFLAGS = -static  # for speed and Valgrind
LDADD = ../src/libjansson.la

TESTS = test-invalid test-valid

clean-local:
	rm -rf testlogs
