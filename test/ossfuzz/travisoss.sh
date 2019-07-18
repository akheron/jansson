#!/bin/bash

set -ex

PROJECT_NAME=jansson

# Clone the oss-fuzz repository
git clone https://github.com/google/oss-fuzz.git /tmp/ossfuzz

if [[ ! -d /tmp/ossfuzz/projects/${PROJECT_NAME} ]]
then
    echo "Could not find the ${PROJECT_NAME} project in ossfuzz"

    # Exit with a success code while the jansson project is not expected to exist
    # on oss-fuzz.
    exit 0
fi

# Modify the oss-fuzz Dockerfile so that we're checking out the current branch on travis.
sed -i "s@https://github.com/akheron/jansson.git@-b $TRAVIS_BRANCH https://github.com/akheron/jansson.git@" /tmp/ossfuzz/projects/${PROJECT_NAME}/Dockerfile

# Try and build the fuzzers
pushd /tmp/ossfuzz
python infra/helper.py build_image --pull ${PROJECT_NAME}
python infra/helper.py build_fuzzers ${PROJECT_NAME}
popd
