#!/bin/bash
set -e

ARG_VAL="$1"
BUILD_DIR="build"
MAKE_DEB=""
[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

[[ "${MSYSTEM}" =~ "MINGW" ]] && IS_MSYS2_BUILD="Y"
[ -n "${IS_MSYS2_BUILD}" ] && DEFAULT_BUILD_TYPE="Release" || DEFAULT_BUILD_TYPE="Debug"

if [ "${ARG_VAL}" == "debug" ] || [ "${ARG_VAL}" == "Debug" ] || [ "${ARG_VAL}" == "DEBUG" ]
then
  CMAKE_BUILD_TYPE="Debug"
elif [ "${ARG_VAL}" == "release" ] || [ "${ARG_VAL}" == "Release" ] || [ "${ARG_VAL}" == "RELEASE" ]
then
  CMAKE_BUILD_TYPE="Release"
else
  CMAKE_BUILD_TYPE=${DEFAULT_BUILD_TYPE}
  if [ "${ARG_VAL}" == "deb" ] || [ "${ARG_VAL}" == "Deb" ] || [ "${ARG_VAL}" == "DEB" ]
  then
    MAKE_DEB="Y"
  fi
fi

if [ -f /etc/lsb-release ]
then
  DISTRIB_ID="$(grep DISTRIB_ID /etc/lsb-release | awk -F= '{print $2}')"
  DISTRIB_RELEASE="$(grep DISTRIB_RELEASE /etc/lsb-release | awk -F= '{print $2}')"
fi

echo "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"

if [ -n "${DISTRIB_ID}" ] && [ -n "${DISTRIB_RELEASE}" ]
then
  echo "Building on ${DISTRIB_ID} ${DISTRIB_RELEASE}"
  if [ "${DISTRIB_ID}${DISTRIB_RELEASE}" == "Ubuntu18.04" ]
  then
    EXTRA_CMAKE_FLAGS="-DUSE_SHARED_FMT_SPDLOG=''"
  fi
fi

cd ${BUILD_DIR}
cmake .. -DCMAKE_C_COMPILER=gcc \
         -DCMAKE_CXX_COMPILER=g++ \
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
         ${EXTRA_CMAKE_FLAGS} -DINSTALL_GTEST='' -GNinja
ninja

if [ -n "${MAKE_DEB}" ]
then
  cpack -G DEB
  PACKAGE_VERSION="$(grep 'PACKAGE_VERSION ' ../config.h | awk -F\" '{print $2}')"
  TARGET_PACKAGE_NAME="cherrytree-${PACKAGE_VERSION}~${DISTRIB_ID}${DISTRIB_RELEASE}_amd64.deb"
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.deb ${TARGET_PACKAGE_NAME}
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.deb.sha256 ${TARGET_PACKAGE_NAME}.sha256
fi
