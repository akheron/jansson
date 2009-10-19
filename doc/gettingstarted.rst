***************
Getting Started
***************

.. highlight:: c

Compiling and Installing Jansson
================================

The Jansson source is available at
http://www.digip.org/jansson/releases/.

Unpack the source tarball and change to the source directory:

.. parsed-literal::

    bunzip2 -c jansson-|release|.tar.bz2 | tar xf -
    cd jansson-|release|

The source uses GNU Autotools (autoconf_, automake_, libtool_), so
compiling and installing is extremely simple::

    ./configure
    make
    make check
    make install

To change the destination directory (``/usr/local`` by default), use
the ``--prefix=DIR`` argument to ``./configure``. See ``./configure
--help`` for the list of all possible installation options. (There are
no options to customize the resulting Jansson binary.)

The command ``make check`` runs the test suite distributed with
Jansson. Python_ is required to run the tests. This step is not
strictly necessary, but it may find possible problems that Jansson has
on your platform. If any problems are found, please report them.

If you obtained the source from a Git repository (or any other source
control system), there's no ``./configure`` script as it's not kept in
version control. To create the script, Autotools needs to be
bootstrapped. There are many ways to do this, but the easiest one is
to use ``autoreconf``::

    autoreconf -vi

This command creates the ``./configure`` script, which can then be
used as described in the previous section.

.. _autoconf: http://www.gnu.org/software/autoconf/
.. _automake: http://www.gnu.org/software/automake/
.. _libtool: http://www.gnu.org/software/libtool/
.. _Python: http://www.python.org/


Installing Prebuilt Binary Packages
-----------------------------------

Binary ``.deb`` packages for Ubuntu are available in `this PPA`_ at
Launchpad_. Follow the instructions in the PPA ("Technical details
about this PPA" link) to take the PPA into use. Then install the -dev
package::

  sudo apt-get install libjansson-dev

.. _this PPA: http://launchpad.net/~petri/+archive/ppa
.. _Launchpad: http://launchpad.net/


Building the Documentation
--------------------------

(This subsection describes how to build the HTML documentation you are
currently reading, so it can be safely skipped.)

Documentation is in the ``doc/`` subdirectory. It's written in
reStructuredText_ with Sphinx_ annotations. To generate the HTML
documentation, invoke::

   cd doc/
   sphinx-build . .build/html

... and point your browser to ``.build/html/index.html``. Sphinx_ is
required to generate the documentation.

.. _reStructuredText: http://docutils.sourceforge.net/rst.html
.. _Sphinx: http://sphinx.pocoo.org/


Compiling Programs Using Jansson
================================

Jansson involves one C header file, :file:`jansson.h`, so it's enough
to put the line

::

    #include <jansson.h>

in the beginning of every source file that uses Jansson.

There's also just one library to link with, ``libjansson``. Compile and
link the program as follows::

    cc -o prog prog.c -ljansson
