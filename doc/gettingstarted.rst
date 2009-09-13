***************
Getting Started
***************

.. highlight:: c

Compiling and Installing Jansson
================================

This chapter explains how to compile and install the library itself.


Compiling and Installing from a Source Tarball
----------------------------------------------

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
Jansson. This step is not strictly necessary, but it may find possible
problems that Jansson has on your platform. If any problems are found,
please report them.

.. _autoconf: http://www.gnu.org/software/autoconf/
.. _automake: http://www.gnu.org/software/automake/
.. _libtool: http://www.gnu.org/software/libtool/


Compiling and Installing from Git
---------------------------------

If you obtained the source from a Git repository (or any other source
control system), there's no ``./configure`` script as it's not kept in
version control. To create the script, Autotools needs to be
bootstrapped. There are many ways to do this, but the easiest one is
to use ``autoreconf``::

    autoreconf -vi

This command creates the ``./configure`` script, which can then be
used as described in the previous section.


Installing Prebuilt Binary Packages
-----------------------------------

Binary ``.deb`` packages for Ubuntu are available in the `Jansson
PPA`_ at Launchpad_. Follow the instructions in the PPA ("Read about
installing" link) to take the PPA into use. Then install the -dev
package::

  apt-get install libjansson-dev

.. _Jansson PPA: http://launchpad.net/~petri/+archive/ppa
.. _Launchpad: http://launchpad.net/



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
