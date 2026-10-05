SUBDIRS = bin suites ossfuzz
EXTRA_DIST = scripts run-suites cmake/package_version.cmake

TESTS = run-suites
TESTS_ENVIRONMENT = \
	top_srcdir=$(top_srcdir) \
	top_builddir=$(top_builddir)

clean-local:
	rm -rf logs
