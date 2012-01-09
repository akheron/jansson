Jansson README
==============

Jansson_ is a C library for encoding, decoding and manipulating JSON
data. Its main features and design principles are:

- Simple and intuitive API and data model

- Comprehensive documentation

- No dependencies on other libraries

- Full Unicode support (UTF-8)

- Extensive test suite

- Unix and Windows support

Jansson is licensed under the `MIT license`_; see LICENSE in the
source distribution for details.


Compilation and Installation
----------------------------

If you obtained a source tarball, just use the standard cmake commands::

   $ mkdir build && cd build
   $ cmake ..
   $ ccmake .  # optional; use this to change the build settings
   $ make install

To run the test suite, invoke::

   $ make check


Documentation
-------------

Prebuilt HTML documentation is available at
http://www.digip.org/jansson/doc/.

To generate HTML documentation, invoke::

   $ make doc

Then, point your browser to ``html/index.html``. Sphinx_
1.0 or newer is required to generate the documentation.


.. _Jansson: http://www.digip.org/jansson/
.. _`MIT license`: http://www.opensource.org/licenses/mit-license.php
.. _Sphinx: http://sphinx.pocoo.org/
