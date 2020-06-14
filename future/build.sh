#!/bin/bash
set -e

[[ "${MSYSTEM}" =~ "MINGW" ]] && IS_MSYS2_BUILD="Y"
[ -n "${IS_MSYS2_BUILD}" ] && DEFAULT_BUILD_TYPE="Release" || DEFAULT_BUILD_TYPE="Debug"
[ -n "$1" ] && CMAKE_BUILD_TYPE="$1" || CMAKE_BUILD_TYPE="${DEFAULT_BUILD_TYPE}"
BUILD_DIR="build"

[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

echo "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
cd ${BUILD_DIR}
if [ -n "${IS_MSYS2_BUILD}" ]
then
  cmake .. -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
else
  cmake .. -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
fi
make -j$(nproc --all)

# run unit tests
./tests/run_tests
