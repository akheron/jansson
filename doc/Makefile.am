EXTRA_DIST = \
	conf.py apiref.rst gettingstarted.rst github_commits.c index.rst \
	tutorial.rst ext/refcounting.py

SPHINXBUILD = sphinx-build
SPHINXOPTS = -d _build/doctrees -W

# Convert json_*() functions to :cfunc: cross references in
# ../CHANGES, and add a header from changes.rst.in
changes.rst: changes.rst.in ../CHANGES
	set -e; \
	cat changes.rst.in >$@; \
	sed -r -e 's/``(json_[a-z_]+\(\))``/:cfunc:`\1`/g' ../CHANGES \
		>>$@

html-local: changes.rst
	$(SPHINXBUILD) -b html $(SPHINXOPTS) $(srcdir) _build/html

install-html-local: html
	mkdir -p $(DESTDIR)$(htmldir)
	cp -r _build/html $(DESTDIR)$(htmldir)

uninstall-local:
	rm -rf $(DESTDIR)$(htmldir)

clean-local:
	rm -rf _build
	rm -f ext/refcounting.pyc changes.rst
