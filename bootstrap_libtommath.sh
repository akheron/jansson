#!/bin/sh

if [ -z "$ACLOCAL" ]
    then ACLOCAL=aclocal
fi
if [ -z "$AUTOMAKE" ]
     then AUTOMAKE=automake
fi
if [ -z "$LIBTOOLIZE" ]
    then LIBTOOLIZE=glibtolize
fi
if [ -z "$AUTOCONF" ]
    then AUTOCONF=autoconf
fi

set -e
git submodule init
git submodule update
(cd libtommath; mv makefile makefile.old; patch -p0 ../tommath_am.patch)
$ACLOCAL
$LIBTOOLIZE
$AUTOMAKE
$AUTOCONF 