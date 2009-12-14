SUBDIRS = bin suites
EXTRA_DIST = scripts

TESTS = run-suites
TESTS_ENVIRONMENT = \
	top_srcdir=$(top_srcdir) \
	top_builddir=$(top_builddir)

clean-local:
	rm -rf logs
