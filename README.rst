Jansson README
==============

.. image:: https://travis-ci.org/akheron/jansson.png
  :alt: Build status
  :target: https://travis-ci.org/akheron/jansson

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
If you're running Ubuntu, you probably need to install additional packages::

   $ sudo apt-get install autoconf libtool

If you obtained a source tarball, just use the standard autotools
commands::

   $ ./configure
   $ make
   $ make install

To run the test suite, invoke::

   $ make check

If the source has been checked out from a Git repository, the
./configure script has to be generated first. The easiest way is to
use autoreconf::

   $ autoreconf -i

Make sure libjansson is cached by `ldconfig`::

   # ldconfig -p | grep jansson
   
If you get no results or an empty line -- then run::

   # ldconfig -n /path/to/libjansson.so.xxxx

Don't forget to link jansson library when compiling::

   $ echo -e "#include <jansson.h>\n\nint main() {\n\tjson_t *obj = json_pack(\"{s:i}\", \"id\", 10002);\n\treturn 0;\n}" > my_jansson_code.c
   $ gcc -ljansson my_jansson_code.c

Documentation
-------------

Prebuilt HTML documentation is available at
http://www.digip.org/jansson/doc/.

The documentation source is in the ``doc/`` subdirectory. To generate
HTML documentation, invoke::

   $ make html

Then, point your browser to ``doc/_build/html/index.html``. Sphinx_
1.0 or newer is required to generate the documentation.


.. _Jansson: http://www.digip.org/jansson/
.. _`MIT license`: http://www.opensource.org/licenses/mit-license.php
.. _Sphinx: http://sphinx.pocoo.org/
