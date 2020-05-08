#!/bin/bash

BUILD_DIR="build"

[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

cd ${BUILD_DIR}
cmake ..
make -j$(nproc --all)

# run unit tests
./tests/run_tests
