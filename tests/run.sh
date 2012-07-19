#!/bin/bash
# This script should be used for running tests.

# Warning: this path will be erased at each launch.
# This must not be inside source package.
TEST_ENV_PATH="/tmp/ridual-test-env"

BUILD_PATH="/tmp/ridual-test-build"

PROJECT_DIR="$PWD/$( dirname "${BASH_SOURCE[0]}" )/.."
rm -rf $TEST_ENV_PATH
cp -R $PROJECT_DIR/tests/env $TEST_ENV_PATH
mkdir -p $BUILD_PATH
cd $BUILD_PATH
qmake $PROJECT_DIR CONFIG+=testing 'DEFINES+="TEST_ENV_PATH=\\\"'$TEST_ENV_PATH'\\\""' &&
make &&
./ridual-test
