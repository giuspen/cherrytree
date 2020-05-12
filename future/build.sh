#!/bin/bash
set -e

BUILD_DIR="build"

[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

cd ${BUILD_DIR}
if [[ "${MSYSTEM}" =~ "^MINGW.*" ]]
then
  cmake .. -G"MSYS Makefiles"
else
  cmake ..
fi
make -j$(nproc --all)

# run unit tests
./tests/run_tests
