#!/bin/bash
set -e
# based on https://gitlab.gnome.org/GNOME/gedit/-/tree/master/build-aux%2Fwin32
# and in particular make-gedit-installer.sh

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
GIT_CT_FOLDER="$(dirname "${SCRIPT_DIR}")"
GIT_CT_EXE="${GIT_CT_FOLDER}/build/cherrytree.exe"
GIT_CT_LANGUAGES_FOLDER="${GIT_CT_FOLDER}/po"
GIT_CT_DATA_FOLDER="${GIT_CT_FOLDER}/data"
GIT_CT_ICONS_FOLDER="${GIT_CT_FOLDER}/icons"
GIT_CT_LANGUAGE_SPECS_FOLDER="${GIT_CT_FOLDER}/language-specs"
GIT_CT_STYLES_FOLDER="${GIT_CT_FOLDER}/styles"
GIT_CT_LICENSE="${GIT_CT_FOLDER}/license.txt"
GIT_CT_HUNSPELL="${GIT_CT_FOLDER}/hunspell"
GIT_CT_CONFIG_H="${GIT_CT_FOLDER}/config.h"
DOWNGRADE_PACKAGE_LOCATION="http://repo.msys2.org/mingw/x86_64"
DOWNGRADE_PACKAGE_NAME=""

if [ -n "${DOWNGRADE_PACKAGE_NAME}" ]
then
  wget "${DOWNGRADE_PACKAGE_LOCATION}/${DOWNGRADE_PACKAGE_NAME}"
  pacman -U --noconfirm ${DOWNGRADE_PACKAGE_NAME}
fi
echo "build..."
cd ${GIT_CT_FOLDER}
./build.sh


CT_VERSION_NUM="$(cat ${GIT_CT_CONFIG_H} | grep PACKAGE_VERSION_WINDOWS_STR | awk '{print substr($3, 2, length($3)-2)}')"
NEW_MSYS2_FOLDER="C:/Users/${USER}/Desktop/cherrytree-msys2"
NEW_ROOT_FOLDER="C:/Users/${USER}/Desktop/cherrytree_${CT_VERSION_NUM}_win64_portable"
NEW_MINGW64_FOLDER="${NEW_ROOT_FOLDER}/mingw64"
NEW_ETC_GTK_FOLDER="${NEW_ROOT_FOLDER}/etc/gtk-3.0"
NEW_ETC_GTK_SETTINGS_INI="${NEW_ETC_GTK_FOLDER}/settings.ini"
NEW_HUNSPELL_FOLDER="${NEW_MINGW64_FOLDER}/share/hunspell"
NEW_CHERRYTREE_SHARE="${NEW_MINGW64_FOLDER}/usr/share/cherrytree"


echo "cleanup old runs..."
for folderpath in ${NEW_MSYS2_FOLDER} ${NEW_ROOT_FOLDER}
do
  [ -d ${folderpath} ] && rm -rfv ${folderpath}
done


echo "installing minimal filesystem to ${NEW_MSYS2_FOLDER}..."
mkdir -p ${NEW_MSYS2_FOLDER}
pushd ${NEW_MSYS2_FOLDER} > /dev/null
mkdir -p var/lib/pacman
mkdir -p var/log
mkdir -p tmp
pacman -Syu --root ${NEW_MSYS2_FOLDER}
pacman -S --noconfirm --root ${NEW_MSYS2_FOLDER} \
  filesystem \
  bash \
  pacman \
  mingw-w64-x86_64-gtkmm3 \
  mingw-w64-x86_64-gtksourceviewmm3 \
  mingw-w64-x86_64-libxml++2.6 \
  mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-gspell \
  mingw-w64-x86_64-curl \
  mingw-w64-x86_64-uchardet \
  mingw-w64-x86_64-fmt \
  mingw-w64-x86_64-spdlog
_result=$?
if [ "$_result" -ne "0" ]; then
  echo "failed to create base data via command 'pacman -S <packages names list> --noconfirm --root ${NEW_MSYS2_FOLDER}'"
  exit 1
fi
if [ -n "${DOWNGRADE_PACKAGE_NAME}" ]
then
  wget "${DOWNGRADE_PACKAGE_LOCATION}/${DOWNGRADE_PACKAGE_NAME}"
  pacman -U --noconfirm --root ${NEW_MSYS2_FOLDER} ${DOWNGRADE_PACKAGE_NAME}
  _result=$?
  if [ "$_result" -ne "0" ]; then
    echo "failed to downgrade package"
    exit 1
  fi
fi

popd > /dev/null


echo "moving over necessary files from ${NEW_MSYS2_FOLDER} to ${NEW_ROOT_FOLDER}..."
mkdir -p ${NEW_ROOT_FOLDER}
_result="1"
while [ "$_result" -ne "0" ]
do
  sleep 1
  mv -v ${NEW_MSYS2_FOLDER}/mingw64 ${NEW_ROOT_FOLDER}/
  _result=$?
done


echo "removing unnecessary files..."
# remove .a files not needed for the installer
find ${NEW_MINGW64_FOLDER} -name "*.a" -exec rm -f {} \;
# remove unneeded binaries
find ${NEW_MINGW64_FOLDER} -not -name "g*.exe" -name "*.exe" -exec rm -f {} \;
rm -rf ${NEW_MINGW64_FOLDER}/bin/2to3*
rm -rf ${NEW_MINGW64_FOLDER}/bin/autopoint
rm -rf ${NEW_MINGW64_FOLDER}/bin/idle*
rm -rf ${NEW_MINGW64_FOLDER}/bin/bz*
rm -rf ${NEW_MINGW64_FOLDER}/bin/xz*
rm -rf ${NEW_MINGW64_FOLDER}/bin/gtk3-*.exe
rm -rf ${NEW_MINGW64_FOLDER}/bin/*gettextize
rm -rf ${NEW_MINGW64_FOLDER}/bin/*.sh
rm -rf ${NEW_MINGW64_FOLDER}/bin/update-*
rm -rf ${NEW_MINGW64_FOLDER}/bin/gdbm*.exe
rm -rf ${NEW_MINGW64_FOLDER}/bin/py*
rm -rf ${NEW_MINGW64_FOLDER}/bin/*-config
rm -f ${NEW_MINGW64_FOLDER}/bin/tcl86.dll
rm -f ${NEW_MINGW64_FOLDER}/bin/tk86.dll
# remove other useless folders/files
rm -rf ${NEW_MINGW64_FOLDER}/var
rm -rf ${NEW_MINGW64_FOLDER}/include
rm -rf ${NEW_MINGW64_FOLDER}/libexec
rm -rf ${NEW_MINGW64_FOLDER}/share/man
rm -rf ${NEW_MINGW64_FOLDER}/share/readline
rm -rf ${NEW_MINGW64_FOLDER}/share/info
rm -rf ${NEW_MINGW64_FOLDER}/share/aclocal
rm -rf ${NEW_MINGW64_FOLDER}/share/gnome-common
rm -rf ${NEW_MINGW64_FOLDER}/share/glade
rm -rf ${NEW_MINGW64_FOLDER}/share/gettext
rm -rf ${NEW_MINGW64_FOLDER}/share/terminfo
rm -rf ${NEW_MINGW64_FOLDER}/share/tabset
rm -rf ${NEW_MINGW64_FOLDER}/share/pkgconfig
rm -rf ${NEW_MINGW64_FOLDER}/share/bash-completion
rm -rf ${NEW_MINGW64_FOLDER}/share/appdata
rm -rf ${NEW_MINGW64_FOLDER}/share/gdb
rm -rf ${NEW_MINGW64_FOLDER}/share/help
rm -rf ${NEW_MINGW64_FOLDER}/share/gtk-doc
rm -rf ${NEW_MINGW64_FOLDER}/share/doc
rm -rf ${NEW_MINGW64_FOLDER}/share/applications
rm -rf ${NEW_MINGW64_FOLDER}/share/devhelp
rm -rf ${NEW_MINGW64_FOLDER}/share/gir-*
rm -rf ${NEW_MINGW64_FOLDER}/share/graphite*
rm -rf ${NEW_MINGW64_FOLDER}/share/installed-tests
rm -rf ${NEW_MINGW64_FOLDER}/share/vala
rm -f ${NEW_MINGW64_FOLDER}/share/sqlite/extensions/*.c
# remove on the lib folder
rm -rf ${NEW_MINGW64_FOLDER}/lib/atkmm*
rm -rf ${NEW_MINGW64_FOLDER}/lib/cairomm*
rm -rf ${NEW_MINGW64_FOLDER}/lib/cmake
rm -rf ${NEW_MINGW64_FOLDER}/lib/dde*
rm -rf ${NEW_MINGW64_FOLDER}/lib/engines*
rm -rf ${NEW_MINGW64_FOLDER}/lib/gdkmm*
rm -rf ${NEW_MINGW64_FOLDER}/lib/gettext
rm -rf ${NEW_MINGW64_FOLDER}/lib/giomm*
rm -rf ${NEW_MINGW64_FOLDER}/lib/girepository*
rm -rf ${NEW_MINGW64_FOLDER}/lib/glib*
rm -rf ${NEW_MINGW64_FOLDER}/lib/gtk*
rm -rf ${NEW_MINGW64_FOLDER}/lib/itcl*
rm -rf ${NEW_MINGW64_FOLDER}/lib/libxml*
rm -rf ${NEW_MINGW64_FOLDER}/lib/pango*
rm -rf ${NEW_MINGW64_FOLDER}/lib/python*
rm -rf ${NEW_MINGW64_FOLDER}/lib/pkgconfig
rm -rf ${NEW_MINGW64_FOLDER}/lib/peas-demo
rm -rf ${NEW_MINGW64_FOLDER}/lib/reg*
rm -rf ${NEW_MINGW64_FOLDER}/lib/sigc*
rm -rf ${NEW_MINGW64_FOLDER}/lib/sqlite*
rm -rf ${NEW_MINGW64_FOLDER}/lib/terminfo
rm -rf ${NEW_MINGW64_FOLDER}/lib/tcl*
rm -rf ${NEW_MINGW64_FOLDER}/lib/tdbc*
rm -rf ${NEW_MINGW64_FOLDER}/lib/thread*
rm -rf ${NEW_MINGW64_FOLDER}/lib/tk*
rm -rf ${NEW_MINGW64_FOLDER}/lib/*.sh

# remove the languages that we are not supporting
LOCALE="${NEW_MINGW64_FOLDER}/share/locale"
LOCALE_TMP="${LOCALE}-tmp"
mkdir ${LOCALE_TMP}
for element_rel in $(ls ${GIT_CT_LANGUAGES_FOLDER})
do
  element_abs=${GIT_CT_LANGUAGES_FOLDER}/${element_rel}
  [ -d ${element_abs} ] && [ -d ${LOCALE}/${element_rel} ] && mv -fv ${LOCALE}/${element_rel} ${LOCALE_TMP}/
done

rm -rf ${LOCALE}
mv ${LOCALE_TMP} ${LOCALE}

# strip the binaries to reduce the size
find ${NEW_MINGW64_FOLDER} -name *.dll | xargs strip
find ${NEW_MINGW64_FOLDER} -name *.exe | xargs strip


echo "set use native windows theme..."
[ -d ${NEW_ETC_GTK_FOLDER} ] || mkdir -p ${NEW_ETC_GTK_FOLDER}
echo "[Settings]" > ${NEW_ETC_GTK_SETTINGS_INI}
echo "gtk-theme-name=win32" >> ${NEW_ETC_GTK_SETTINGS_INI}


echo "copying cherrytree files..."
# exe
strip ${GIT_CT_EXE}
cp -v ${GIT_CT_EXE} ${NEW_MINGW64_FOLDER}/bin/
# license
cp -v ${GIT_CT_LICENSE} ${NEW_ROOT_FOLDER}/
# share data
mkdir -p ${NEW_CHERRYTREE_SHARE}/data
cp -rv ${GIT_CT_LANGUAGE_SPECS_FOLDER} ${NEW_CHERRYTREE_SHARE}/
cp -rv ${GIT_CT_STYLES_FOLDER} ${NEW_CHERRYTREE_SHARE}/
for element_rel in script3.js styles3.css user-style.xml
do
  cp -v ${GIT_CT_DATA_FOLDER}/${element_rel} ${NEW_CHERRYTREE_SHARE}/data/
done
# share icons
mkdir -p ${NEW_CHERRYTREE_SHARE}/icons
cp -v ${GIT_CT_ICONS_FOLDER}/ct_home.svg ${NEW_CHERRYTREE_SHARE}/icons/
# i18n languages
for element_rel in $(ls ${GIT_CT_LANGUAGES_FOLDER})
do
  element_abs=${GIT_CT_LANGUAGES_FOLDER}/${element_rel}
  [ -d ${element_abs} ] && cp -rfv ${element_abs} ${LOCALE}/
done
# spell check languages
mkdir -p ${NEW_HUNSPELL_FOLDER}
cp -v ${GIT_CT_HUNSPELL}/*.aff ${NEW_HUNSPELL_FOLDER}/
cp -v ${GIT_CT_HUNSPELL}/*.dic ${NEW_HUNSPELL_FOLDER}/
