#!/bin/bash
set -e

[ -n "$1" ] && CMAKE_BUILD_TYPE="$1" || CMAKE_BUILD_TYPE="Debug"
BUILD_DIR="build"

[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

cd ${BUILD_DIR}
if [[ "${MSYSTEM}" =~ "MINGW" ]]
then
  cmake .. -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
else
  cmake .. -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
fi
make -j$(nproc --all)

# run unit tests
./tests/run_tests
