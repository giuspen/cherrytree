#!/bin/bash
set -e

ARG1_VAL_LOWER="$(echo ${1} | tr [:upper:] [:lower:])"
ARG2_VAL_LOWER="$(echo ${2} | tr [:upper:] [:lower:])"
ARG3_VAL_LOWER="$(echo ${3} | tr [:upper:] [:lower:])"
BUILD_DIR="build"
MAKE_DEB=""
MAKE_RPM=""
MAKE_APPIMAGE=""
BUNDLED_SPDLOG_FMT=""
NO_TESTS=""
RET_VAL=""
[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

[[ "${MSYSTEM}" =~ "MINGW" ]] && IS_MSYS2_BUILD="Y"
[ -n "${IS_MSYS2_BUILD}" ] && DEFAULT_BUILD_TYPE="Release" || DEFAULT_BUILD_TYPE="Debug"

f_any_argument_matches () {
  if [ "${ARG1_VAL_LOWER}" == "$1" ] || [ "${ARG2_VAL_LOWER}" == "$1" ] || [ "${ARG3_VAL_LOWER}" == "$1" ]
  then
    RET_VAL="1"
  elif [ -n "$2" ] && ( [ "${ARG1_VAL_LOWER}" == "$2" ] || [ "${ARG2_VAL_LOWER}" == "$2" ] || [ "${ARG3_VAL_LOWER}" == "$2" ] )
  then
    RET_VAL="2"
  elif [ -n "$3" ] && ( [ "${ARG1_VAL_LOWER}" == "$3" ] || [ "${ARG2_VAL_LOWER}" == "$3" ] || [ "${ARG3_VAL_LOWER}" == "$3" ] )
  then
    RET_VAL="3"
  else
    RET_VAL=""
  fi
}

f_any_argument_matches "help" "--help" "-h"
if [ -n "${RET_VAL}" ]
then
  echo "$0 [dbg|debug|rel|release] [notest|notests] [bundledspdfmt] [deb|debian] [rpm] [appimage]"
  exit 0
fi

f_any_argument_matches "debug" "dbg"
if [ -n "${RET_VAL}" ]
then
  CMAKE_BUILD_TYPE="Debug"
else
  f_any_argument_matches "release" "rel"
  [ -n "${RET_VAL}" ] && CMAKE_BUILD_TYPE="Release" || CMAKE_BUILD_TYPE=${DEFAULT_BUILD_TYPE}
fi

f_any_argument_matches "notests" "notest"
[ -n "${RET_VAL}" ] && NO_TESTS="Y"

f_any_argument_matches "deb" "debian"
[ -n "${RET_VAL}" ] && MAKE_DEB="Y"

f_any_argument_matches "rpm"
[ -n "${RET_VAL}" ] && MAKE_RPM="Y"

f_any_argument_matches "appimage" "appimg"
[ -n "${RET_VAL}" ] && MAKE_APPIMAGE="Y"

f_any_argument_matches "bundledspdlog" "bundledfmt" "bundledspdfmt"
[ -n "${RET_VAL}" ] && BUNDLED_SPDLOG_FMT="Y"

if [ -f /etc/lsb-release ]
then
  DISTRIB_ID="$(grep DISTRIB_ID /etc/lsb-release | awk -F= '{print $2}')"
  DISTRIB_RELEASE="$(grep DISTRIB_RELEASE /etc/lsb-release | awk -F= '{print $2}')"
elif [ -f /etc/debian_version ]
then
  DISTRIB_ID="Debian"
  DISTRIB_RELEASE="$(cat /etc/debian_version | awk -F. '{print $1}' | tr -d '\n')"
elif [ -f /etc/fedora-release ]
then
  DISTRIB_ID="Fedora"
  DISTRIB_RELEASE="$(cat /etc/fedora-release | awk '{print $3}' | tr -d '\n')"
elif [ -f /etc/os-release ]
then
  DISTRIB_ID="$(grep ^ID= /etc/os-release | awk -F\" '{print $2}')"
  DISTRIB_RELEASE="$(grep ^VERSION_ID= /etc/os-release | awk -F\" '{print $2}')"
fi

echo "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"

if [ -n "${DISTRIB_ID}" ] && [ -n "${DISTRIB_RELEASE}" ]
then
  echo "Building on ${DISTRIB_ID} ${DISTRIB_RELEASE}"
  if [ "${DISTRIB_ID}${DISTRIB_RELEASE}" == "Ubuntu18.04" ] || [ "${DISTRIB_ID}${DISTRIB_RELEASE}" == "Debian10" ]
  then
    BUNDLED_SPDLOG_FMT="Y"
  fi
fi

[ -n "${BUNDLED_SPDLOG_FMT}" ] && EXTRA_CMAKE_FLAGS="${EXTRA_CMAKE_FLAGS} -DUSE_SHARED_FMT_SPDLOG=''"

[[ "$OSTYPE" == "darwin"* ]] && export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig" && NO_TESTS="Y"

if [ -n "${NO_TESTS}" ]
then
  EXTRA_CMAKE_FLAGS="${EXTRA_CMAKE_FLAGS} -DBUILD_TESTING=''"
else
  EXTRA_CMAKE_FLAGS="${EXTRA_CMAKE_FLAGS} -DINSTALL_GTEST=''"
fi

cd ${BUILD_DIR}
cmake .. -DCMAKE_C_COMPILER=gcc \
         -DCMAKE_CXX_COMPILER=g++ \
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
         ${EXTRA_CMAKE_FLAGS} -GNinja
[[ "$OSTYPE" == "darwin"* ]] && NUM_JOBS="$(sysctl -n hw.ncpu)" || NUM_JOBS="$(nproc --all)"
echo "Starting ninja build with up to ${NUM_JOBS} parallel jobs..."
ninja -j ${NUM_JOBS}

if [ -n "${MAKE_DEB}" ]
then
  cpack -G DEB
  PACKAGE_VERSION="$(grep 'PACKAGE_VERSION ' ../config.h | awk -F\" '{print $2}')"
  TARGET_PACKAGE_NAME="cherrytree-${PACKAGE_VERSION}~${DISTRIB_ID}${DISTRIB_RELEASE}_amd64.deb"
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.deb ${TARGET_PACKAGE_NAME}
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.deb.sha256 ${TARGET_PACKAGE_NAME}.sha256
fi
if [ -n "${MAKE_RPM}" ]
then
  cpack -G RPM
  PACKAGE_VERSION="$(grep 'PACKAGE_VERSION ' ../config.h | awk -F\" '{print $2}')"
  TARGET_PACKAGE_NAME="cherrytree-${PACKAGE_VERSION}~${DISTRIB_ID}${DISTRIB_RELEASE}_amd64.rpm"
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.rpm ${TARGET_PACKAGE_NAME}
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.rpm.sha256 ${TARGET_PACKAGE_NAME}.sha256
fi
if [ -n "${MAKE_APPIMAGE}" ]
then
  # https://github.com/linuxdeploy/linuxdeploy-plugin-gtk
  [ -f linuxdeploy-plugin-gtk.sh ] || wget -c "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh"
  [ -f linuxdeploy-x86_64.AppImage ] || wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
  [ -f TinyTeX-1-v2022.04.04.tar.xz ] || wget -c "https://www.giuspen.net/software/TinyTeX-1-v2022.04.04.tar.xz"
  chmod +x linuxdeploy-plugin-gtk.sh
  chmod +x linuxdeploy-x86_64.AppImage
  rm -rf AppDir
  mkdir AppDir
  tar xf TinyTeX-1-v2022.04.04.tar.xz -C AppDir/
  mv AppDir/.TinyTeX AppDir/usr
  DESTDIR=AppDir ninja install
  ./linuxdeploy-x86_64.AppImage \
        --appdir AppDir \
        --plugin gtk \
        --output appimage \
        --icon-file ../icons/cherrytree.svg \
        --desktop-file ../data/cherrytree.desktop
fi
