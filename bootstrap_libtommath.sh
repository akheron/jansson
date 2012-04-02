#!/bin/sh

set -e
git submodule init
git submodule update
(cd libtommath; mv makefile makefile.old; patch -p0 ../tommath_am.patch)
./autoreconf -i
