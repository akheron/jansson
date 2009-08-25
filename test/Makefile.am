DIST_SUBDIRS = testprogs testdata
SUBDIRS = testprogs

check_PROGRAMS = loadf_dumpf loads_dumps load_file_dump_file

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS = -Wall -Werror
LDFLAGS = -static  # for speed and Valgrind
LDADD = ../src/libjansson.la

TESTS = test-api test-invalid test-valid

EXTRA_DIST = \
	test-api \
	test-invalid \
	test-valid \
	run-test \
	json-compare.py \
	split-testfile.py

clean-local:
	rm -rf testlogs
