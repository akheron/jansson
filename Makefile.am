EXTRA_DIST = CHANGES LICENSE README.rst
SUBDIRS = doc src test

distcheck-hook:
	sphinx-build -b html -W \
		$(distdir)/doc \
		$(distdir)/_build/doc/.build/html
