EXTRA_DIST = CHANGES LICENSE README.rst
SUBDIRS = doc src test

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jansson.pc

distcheck-hook:
	sphinx-build -b html -W \
		$(distdir)/doc \
		$(distdir)/_build/doc/.build/html
