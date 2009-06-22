include_HEADERS = jansson.h

lib_LTLIBRARIES = libjansson.la
libjansson_la_SOURCES = \
	dump.c \
	hashtable.c \
	hashtable.h \
	load.c \
	strbuffer.c \
	strbuffer.h \
	util.h \
	value.c
libjansson_la_LDFLAGS = -version-info 0:0:0

AM_CFLAGS = -Wall -Wextra -Werror -std=c99
