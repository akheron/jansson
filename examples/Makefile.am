EXTRA_PROGRAMS = simple_parse

simple_parse_SOURCES = simple_parse.c

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src
LDFLAGS = -static  # for easier running
LDADD = $(top_builddir)/src/libjansson.la

all: simple_parse
