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
  cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_MAKE_PROGRAM=mingw32-make.exe -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G"MSYS Makefiles"
  mingw32-make -j$(nproc --all)
else
  cmake .. -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  make -j$(nproc --all)
fi
