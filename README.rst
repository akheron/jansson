Jansson README
==============

Jansson_ is a C library for encoding, decoding and manipulating JSON
data. Its main features and design principles are:

- Simple and intuitive API and data model

- Comprehensive documentation

- No dependencies on other libraries

- Full Unicode support (UTF-8)

- Extensive test suite

Jansson is licensed under the `MIT license`_; see LICENSE in the
source distribution for details.


Compilation and Installation
----------------------------

If you obtained a source tarball, just use the standard autotools
commands

   $ ./configure
   $ make
   $ make install

To run the test suite, invoke::

   $ make check

If the source has been checked out from a Git repository, the
./configure script has to be generated fist. The easiest way is to use
autoreconf::

   $ autoreconf -i


Documentation
-------------

Prebuilt HTML documentation is available at
http://www.digip.org/jansson/doc/.

The documentation source is in the ``doc/`` subdirectory. To generate
HTML documentation, invoke::

   $ make html

Then, point your browser to ``doc/_build/html/index.html``. Sphinx_
1.0 or newer is required to generate the documentation.

csm's Changes
-------------

My fork of this library currently adds two features to jansson that I
needed for my usage:

1. Preservation of insertion order in JSON objects, which also means that
jansson will now preserve the order of object properties from objects it
reads.

2. Adds bignum support for integers larger than json_int_t via the libtommath
library.

The addendum to the compilation section above is that you'll need to set up
libtommath by using the "boottrap_libtommath.sh" script.

We did send a pull request for these features, but aren't bullish about
getting them merged into master (and, relying on libtommath *does* add
a dependency to this library, which some may dislike).


.. _Jansson: http://www.digip.org/jansson/
.. _`MIT license`: http://www.opensource.org/licenses/mit-license.php
.. _Sphinx: http://sphinx.pocoo.org/
