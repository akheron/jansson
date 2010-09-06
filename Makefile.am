EXTRA_DIST = CHANGES LICENSE README.rst
SUBDIRS = doc src test

check-doc:
	$(MAKE) SPHINXOPTS_EXTRA=-W html

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = jansson.pc
