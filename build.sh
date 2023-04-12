#!/bin/bash
set -e

ARG_VAL_LOWER="$(echo ${1} | tr [:upper:] [:lower:])"
BUILD_DIR="build"
MAKE_DEB=""
MAKE_RPM=""
MAKE_APPIMAGE=""
NO_TESTS=""
[ -d ${BUILD_DIR} ] || mkdir ${BUILD_DIR}

[[ "${MSYSTEM}" =~ "MINGW" ]] && IS_MSYS2_BUILD="Y"
[ -n "${IS_MSYS2_BUILD}" ] && DEFAULT_BUILD_TYPE="Release" || DEFAULT_BUILD_TYPE="Debug"

if [ "${ARG_VAL_LOWER}" == "debug" ] || [ "${ARG_VAL_LOWER}" == "dbg" ]
then
  CMAKE_BUILD_TYPE="Debug"
elif [ "${ARG_VAL_LOWER}" == "release" ] || [ "${ARG_VAL_LOWER}" == "rel" ]
then
  CMAKE_BUILD_TYPE="Release"
else
  CMAKE_BUILD_TYPE=${DEFAULT_BUILD_TYPE}
  if [ "${ARG_VAL_LOWER}" == "notests" ]
  then
    NO_TESTS="Y"
  elif [ "${ARG_VAL_LOWER}" == "deb" ]
  then
    MAKE_DEB="Y"
  elif [ "${ARG_VAL_LOWER}" == "rpm" ]
  then
    MAKE_RPM="Y"
  elif [ "${ARG_VAL_LOWER}" == "appimage" ]
  then
    MAKE_APPIMAGE="Y"
  fi
fi

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
    EXTRA_CMAKE_FLAGS="${EXTRA_CMAKE_FLAGS} -DUSE_SHARED_FMT_SPDLOG=''"
  fi
fi

[[ "$OSTYPE" == "darwin"* ]] && export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig" && NO_TESTS="Y"

[ -n "${NO_TESTS}" ] && EXTRA_CMAKE_FLAGS="${EXTRA_CMAKE_FLAGS} -DBUILD_TESTING=''"

cd ${BUILD_DIR}
cmake .. -DCMAKE_C_COMPILER=gcc \
         -DCMAKE_CXX_COMPILER=g++ \
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
         ${EXTRA_CMAKE_FLAGS} -DINSTALL_GTEST='' -GNinja
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
elif [ -n "${MAKE_RPM}" ]
then
  cpack -G RPM
  PACKAGE_VERSION="$(grep 'PACKAGE_VERSION ' ../config.h | awk -F\" '{print $2}')"
  TARGET_PACKAGE_NAME="cherrytree-${PACKAGE_VERSION}~${DISTRIB_ID}${DISTRIB_RELEASE}_amd64.rpm"
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.rpm ${TARGET_PACKAGE_NAME}
  mv -v cherrytree-${PACKAGE_VERSION}-Linux.rpm.sha256 ${TARGET_PACKAGE_NAME}.sha256
elif [ -n "${MAKE_APPIMAGE}" ]
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
