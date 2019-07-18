#!/bin/bash -eu

# This script is called by the oss-fuzz main project when compiling the fuzz
# targets. This script is regression tested by travisoss.sh.

# Save off the current folder as the build root.
export BUILD_ROOT=$PWD

echo "CC: $CC"
echo "CXX: $CXX"
echo "LIB_FUZZING_ENGINE: $LIB_FUZZING_ENGINE"
echo "CFLAGS: $CFLAGS"
echo "CXXFLAGS: $CXXFLAGS"
echo "OUT: $OUT"

export MAKEFLAGS+="-j$(nproc)"

# Install dependencies
apt-get -y install automake libtool

# Compile the fuzzer.
autoreconf -i
./configure --enable-ossfuzzers
make

# Copy the fuzzer to the output directory.
cp -v test/ossfuzz/json_load_dump_fuzzer $OUT/
