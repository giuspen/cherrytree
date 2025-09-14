#!/bin/bash
set -e
# based on https://gitlab.gnome.org/GNOME/gedit/-/tree/master/build-aux%2Fwin32
# and in particular make-gedit-installer.sh

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
IN_CT_FOLDER="$(dirname "${SCRIPT_DIR}")"
IN_CT_EXE="${IN_CT_FOLDER}/build/cherrytree.exe"
IN_CT_LANGUAGES_FOLDER="${IN_CT_FOLDER}/po"
IN_CT_DATA_FOLDER="${IN_CT_FOLDER}/data"
IN_CT_ICONS_FOLDER="${IN_CT_FOLDER}/icons"
IN_CT_LANGUAGE_SPECS_FOLDER="${IN_CT_FOLDER}/language-specs"
IN_CT_STYLES_FOLDER="${IN_CT_FOLDER}/styles"
IN_CT_LICENSE="${IN_CT_FOLDER}/license.txt"
IN_CT_HUNSPELL="${IN_CT_FOLDER}/hunspell"
IN_CT_CONFIG_H="${IN_CT_FOLDER}/config.h"
OLD_UCRT64_FOLDER="/ucrt64"
DOWNGRADE_PACKAGES_FOLDER_URL="https://repo.msys2.org/mingw/ucrt64"
DOWNGRADE_PACKAGES_FILENAMES=("mingw-w64-ucrt-x86_64-cairo-1.18.4-1-any.pkg.tar.zst" "mingw-w64-ucrt-x86_64-pango-1.56.3-2-any.pkg.tar.zst")

for package_name in ${DOWNGRADE_PACKAGES_FILENAMES[@]}; do
  wget ${DOWNGRADE_PACKAGES_FOLDER_URL}/${package_name}
  pacman -U --noconfirm ${package_name}
  _result=$?
  if [ "$_result" -ne "0" ]; then
    echo "failed to downgrade package"
    exit 1
  fi
done
echo "build..."
cd ${IN_CT_FOLDER}
./build.sh release notest


CT_VERSION_NUM="$(cat ${IN_CT_CONFIG_H} | grep PACKAGE_VERSION_WINDOWS_STR | awk '{print substr($3, 2, length($3)-2)}')"
OUT_MSYS2_FOLDER="${IN_CT_FOLDER}/build/cherrytree-msys2"
OUT_ROOT_FOLDER="${IN_CT_FOLDER}/build/cherrytree_${CT_VERSION_NUM}_win64_portable"
OUT_ROOT_FOLDER_NOLATEX="${OUT_ROOT_FOLDER}_nolatex"
OUT_UCRT64_FOLDER="${OUT_ROOT_FOLDER}/ucrt64"
OUT_ETC_GTK_FOLDER="${OUT_ROOT_FOLDER}/etc/gtk-3.0"
OUT_ETC_GTK_SETTINGS_INI="${OUT_ETC_GTK_FOLDER}/settings.ini"
OUT_HUNSPELL_FOLDER="${OUT_UCRT64_FOLDER}/share/hunspell"
OUT_CHERRYTREE_SHARE="${OUT_UCRT64_FOLDER}/usr/share/cherrytree"


# latex.exe and dvipng.exe ensure the list of files to copy from is available
for element_rel in latex.exe \
                   libkpathsea-6.dll \
                   mktexfmt.exe \
                   runscript.dll \
                   runscript.tlu
do
  ls -la ${OLD_UCRT64_FOLDER}/bin/${element_rel}
done
ls -la ${OLD_UCRT64_FOLDER}/var/lib/texmf/web2c/pdftex/latex.fmt
ls -la ${OLD_UCRT64_FOLDER}/share/texmf-dist
for element_rel in dvipng.exe \
                   libgd.dll \
                   libheif.dll \
                   libavif-16.dll \
                   libimagequant.dll \
                   libXpm-noX4.dll \
                   libaom.dll \
                   libdav1d-7.dll \
                   librav1e.dll \
                   libde265-0.dll \
                   libx265-215.dll \
                   libSvtAv1Enc-3.dll \
                   libyuv.dll \
                   libopenjp2-7.dll \
                   libopenjph-0.23.dll \
                   libopenh264-7.dll \
                   libkvazaar-7.dll \
                   libcryptopp.dll
do
  ls -la ${OLD_UCRT64_FOLDER}/bin/${element_rel}
done
ls -la ${OLD_UCRT64_FOLDER}/var/lib/texmf/fonts/map/dvips/updmap/ps2pk.map


echo "cleanup old runs..."
for folderpath in ${OUT_MSYS2_FOLDER} ${OUT_ROOT_FOLDER}
do
  [ -d ${folderpath} ] && rm -rfv ${folderpath}
done


echo "installing minimal filesystem to ${OUT_MSYS2_FOLDER}..."
mkdir -p ${OUT_MSYS2_FOLDER}
pushd ${OUT_MSYS2_FOLDER} > /dev/null
mkdir -p var/lib/pacman
mkdir -p var/log
mkdir -p tmp
pacman -Syu --root ${OUT_MSYS2_FOLDER}
pacman -S --noconfirm --root ${OUT_MSYS2_FOLDER} \
  filesystem \
  bash \
  pacman \
  mingw-w64-ucrt-x86_64-gtkmm3 \
  mingw-w64-ucrt-x86_64-gtksourceview4 \
  mingw-w64-ucrt-x86_64-libxml++2.6 \
  mingw-w64-ucrt-x86_64-sqlite3 \
  mingw-w64-ucrt-x86_64-gspell \
  mingw-w64-ucrt-x86_64-curl \
  mingw-w64-ucrt-x86_64-uchardet \
  mingw-w64-ucrt-x86_64-fribidi \
  mingw-w64-ucrt-x86_64-fmt \
  mingw-w64-ucrt-x86_64-spdlog
_result=$?
if [ "$_result" -ne "0" ]; then
  echo "failed to create base data via command 'pacman -S <packages names list> --noconfirm --root ${OUT_MSYS2_FOLDER}'"
  exit 1
fi
for package_name in ${DOWNGRADE_PACKAGES_FILENAMES[@]}; do
  wget ${DOWNGRADE_PACKAGES_FOLDER_URL}/${package_name}
  pacman -U --noconfirm --root ${OUT_MSYS2_FOLDER} ${package_name}
  _result=$?
  if [ "$_result" -ne "0" ]; then
    echo "failed to downgrade package"
    exit 1
  fi
done

popd > /dev/null


echo "moving over necessary files from ${OUT_MSYS2_FOLDER} to ${OUT_ROOT_FOLDER}..."
mkdir -p ${OUT_ROOT_FOLDER}
_result="1"
while [ "$_result" -ne "0" ]
do
  sleep 1
  mv -v ${OUT_MSYS2_FOLDER}/ucrt64 ${OUT_ROOT_FOLDER}/
  _result=$?
done


echo "removing unnecessary files..."
# remove .a files not needed for the installer
find ${OUT_UCRT64_FOLDER} -name "*.a" -exec rm -f {} \;
# remove unneeded binaries
find ${OUT_UCRT64_FOLDER} -not -name "g*.exe" -name "*.exe" -exec rm -f {} \;
rm -rf ${OUT_UCRT64_FOLDER}/bin/2to3*
rm -rf ${OUT_UCRT64_FOLDER}/bin/autopoint
rm -rf ${OUT_UCRT64_FOLDER}/bin/idle*
rm -rf ${OUT_UCRT64_FOLDER}/bin/bz*
rm -rf ${OUT_UCRT64_FOLDER}/bin/xz*
rm -rf ${OUT_UCRT64_FOLDER}/bin/gtk3-*.exe
rm -rf ${OUT_UCRT64_FOLDER}/bin/*gettextize
rm -rf ${OUT_UCRT64_FOLDER}/bin/*.sh
rm -rf ${OUT_UCRT64_FOLDER}/bin/update-*
rm -rf ${OUT_UCRT64_FOLDER}/bin/gdbm*.exe
rm -rf ${OUT_UCRT64_FOLDER}/bin/py*
rm -rf ${OUT_UCRT64_FOLDER}/bin/*-config
rm -f ${OUT_UCRT64_FOLDER}/bin/tcl86.dll
rm -f ${OUT_UCRT64_FOLDER}/bin/tk86.dll
rm -rf ${OUT_UCRT64_FOLDER}/sbin
# remove other useless folders/files
rm -rf ${OUT_UCRT64_FOLDER}/var
rm -rf ${OUT_UCRT64_FOLDER}/include
rm -rf ${OUT_UCRT64_FOLDER}/libexec
rm -rf ${OUT_UCRT64_FOLDER}/share/man
rm -rf ${OUT_UCRT64_FOLDER}/share/readline
rm -rf ${OUT_UCRT64_FOLDER}/share/info
rm -rf ${OUT_UCRT64_FOLDER}/share/aclocal
rm -rf ${OUT_UCRT64_FOLDER}/share/gnome-common
rm -rf ${OUT_UCRT64_FOLDER}/share/glade
rm -rf ${OUT_UCRT64_FOLDER}/share/gettext
rm -rf ${OUT_UCRT64_FOLDER}/share/terminfo
rm -rf ${OUT_UCRT64_FOLDER}/share/tabset
rm -rf ${OUT_UCRT64_FOLDER}/share/pkgconfig
rm -rf ${OUT_UCRT64_FOLDER}/share/bash-completion
rm -rf ${OUT_UCRT64_FOLDER}/share/appdata
rm -rf ${OUT_UCRT64_FOLDER}/share/gdb
rm -rf ${OUT_UCRT64_FOLDER}/share/help
rm -rf ${OUT_UCRT64_FOLDER}/share/gtk-doc
rm -rf ${OUT_UCRT64_FOLDER}/share/doc
rm -rf ${OUT_UCRT64_FOLDER}/share/applications
rm -rf ${OUT_UCRT64_FOLDER}/share/devhelp
rm -rf ${OUT_UCRT64_FOLDER}/share/gir-*
rm -rf ${OUT_UCRT64_FOLDER}/share/graphite*
rm -rf ${OUT_UCRT64_FOLDER}/share/installed-tests
rm -rf ${OUT_UCRT64_FOLDER}/share/vala
rm -f ${OUT_UCRT64_FOLDER}/share/sqlite/extensions/*.c
# remove on the lib folder
rm -rf ${OUT_UCRT64_FOLDER}/lib/atkmm*
rm -rf ${OUT_UCRT64_FOLDER}/lib/cairomm*
rm -rf ${OUT_UCRT64_FOLDER}/lib/cmake
rm -rf ${OUT_UCRT64_FOLDER}/lib/dde*
rm -rf ${OUT_UCRT64_FOLDER}/lib/engines*
rm -rf ${OUT_UCRT64_FOLDER}/lib/gdkmm*
rm -rf ${OUT_UCRT64_FOLDER}/lib/gettext
rm -rf ${OUT_UCRT64_FOLDER}/lib/giomm*
rm -rf ${OUT_UCRT64_FOLDER}/lib/girepository*
rm -rf ${OUT_UCRT64_FOLDER}/lib/glib*
rm -rf ${OUT_UCRT64_FOLDER}/lib/gtk*
rm -rf ${OUT_UCRT64_FOLDER}/lib/itcl*
rm -rf ${OUT_UCRT64_FOLDER}/lib/libxml*
rm -rf ${OUT_UCRT64_FOLDER}/lib/pango*
rm -rf ${OUT_UCRT64_FOLDER}/lib/python*
rm -rf ${OUT_UCRT64_FOLDER}/lib/pkgconfig
rm -rf ${OUT_UCRT64_FOLDER}/lib/peas-demo
rm -rf ${OUT_UCRT64_FOLDER}/lib/reg*
rm -rf ${OUT_UCRT64_FOLDER}/lib/sigc*
rm -rf ${OUT_UCRT64_FOLDER}/lib/sqlite*
rm -rf ${OUT_UCRT64_FOLDER}/lib/terminfo
rm -rf ${OUT_UCRT64_FOLDER}/lib/tcl*
rm -rf ${OUT_UCRT64_FOLDER}/lib/tdbc*
rm -rf ${OUT_UCRT64_FOLDER}/lib/thread*
rm -rf ${OUT_UCRT64_FOLDER}/lib/tk*
rm -rf ${OUT_UCRT64_FOLDER}/lib/*.sh

# remove the languages that we are not supporting
LOCALE="${OUT_UCRT64_FOLDER}/share/locale"
LOCALE_TMP="${LOCALE}-tmp"
mkdir ${LOCALE_TMP}
for element_rel in $(ls ${IN_CT_LANGUAGES_FOLDER})
do
  element_abs=${IN_CT_LANGUAGES_FOLDER}/${element_rel}
  [ -d ${element_abs} ] && [ -d ${LOCALE}/${element_rel} ] && mv -fv ${LOCALE}/${element_rel} ${LOCALE_TMP}/
done

rm -rf ${LOCALE}
mv ${LOCALE_TMP} ${LOCALE}

# strip the binaries to reduce the size
find ${OUT_UCRT64_FOLDER} -name *.dll | xargs strip
find ${OUT_UCRT64_FOLDER} -name *.exe | xargs strip


echo "set use native windows theme..."
[ -d ${OUT_ETC_GTK_FOLDER} ] || mkdir -p ${OUT_ETC_GTK_FOLDER}
echo "[Settings]" > ${OUT_ETC_GTK_SETTINGS_INI}
echo "gtk-theme-name=win32" >> ${OUT_ETC_GTK_SETTINGS_INI}


echo "copying cherrytree files..."
# exe
strip ${IN_CT_EXE}
cp -v ${IN_CT_EXE} ${OUT_UCRT64_FOLDER}/bin/
# license
cp -v ${IN_CT_LICENSE} ${OUT_ROOT_FOLDER}/
# share data
mkdir -p ${OUT_CHERRYTREE_SHARE}/data
cp -rv ${IN_CT_LANGUAGE_SPECS_FOLDER} ${OUT_CHERRYTREE_SHARE}/
cp -rv ${IN_CT_STYLES_FOLDER} ${OUT_CHERRYTREE_SHARE}/
for element_rel in script3.js \
                   styles4.css \
                   user-style.xml
do
  cp -v ${IN_CT_DATA_FOLDER}/${element_rel} ${OUT_CHERRYTREE_SHARE}/data/
done
# share icons
mkdir -p ${OUT_CHERRYTREE_SHARE}/icons
cp -v ${IN_CT_ICONS_FOLDER}/ct_home.svg ${OUT_CHERRYTREE_SHARE}/icons/
cp -r -v ${IN_CT_ICONS_FOLDER}/Breeze_Dark_icons ${OUT_CHERRYTREE_SHARE}/icons/
cp -r -v ${IN_CT_ICONS_FOLDER}/Breeze_Light_icons ${OUT_CHERRYTREE_SHARE}/icons/
# i18n languages
for element_rel in $(ls ${IN_CT_LANGUAGES_FOLDER})
do
  element_abs=${IN_CT_LANGUAGES_FOLDER}/${element_rel}
  [ -d ${element_abs} ] && cp -rfv ${element_abs} ${LOCALE}/
done
# spell check languages
mkdir -p ${OUT_HUNSPELL_FOLDER}
cp -v ${IN_CT_HUNSPELL}/*.aff ${OUT_HUNSPELL_FOLDER}/
cp -v ${IN_CT_HUNSPELL}/*.dic ${OUT_HUNSPELL_FOLDER}/
# fix issue with ucrt64/etc/ssl not installed properly in --root ${OUT_MSYS2_FOLDER}
rm -rf ${OUT_UCRT64_FOLDER}/etc/ssl
cp -rv ${OLD_UCRT64_FOLDER}/etc/ssl ${OUT_UCRT64_FOLDER}/etc/
# nolatex folder
cp -rv ${OUT_ROOT_FOLDER} ${OUT_ROOT_FOLDER_NOLATEX}
# latex.exe
for element_rel in latex.exe \
                   libkpathsea-6.dll \
                   mktexfmt.exe \
                   runscript.dll \
                   runscript.tlu
do
  cp -v ${OLD_UCRT64_FOLDER}/bin/${element_rel} ${OUT_UCRT64_FOLDER}/bin/
done
cp -v ${OLD_UCRT64_FOLDER}/var/lib/texmf/web2c/pdftex/latex.fmt ${OUT_UCRT64_FOLDER}/bin/
cp -rv ${OLD_UCRT64_FOLDER}/share/texmf-dist ${OUT_UCRT64_FOLDER}/share/
# dvipng.exe
for element_rel in dvipng.exe \
                   libgd.dll \
                   libheif.dll \
                   libavif-16.dll \
                   libimagequant.dll \
                   libXpm-noX4.dll \
                   libaom.dll \
                   libdav1d-7.dll \
                   librav1e.dll \
                   libde265-0.dll \
                   libx265-215.dll \
                   libSvtAv1Enc-3.dll \
                   libyuv.dll \
                   libopenjp2-7.dll \
                   libopenjph-0.23.dll \
                   libopenh264-7.dll \
                   libkvazaar-7.dll \
                   libcryptopp.dll
do
  cp -v ${OLD_UCRT64_FOLDER}/bin/${element_rel} ${OUT_UCRT64_FOLDER}/bin/
done
cp -v ${OLD_UCRT64_FOLDER}/var/lib/texmf/fonts/map/dvips/updmap/ps2pk.map ${OUT_UCRT64_FOLDER}/bin/

7za a ${OUT_ROOT_FOLDER}.7z ${OUT_ROOT_FOLDER}
7za a ${OUT_ROOT_FOLDER_NOLATEX}.7z ${OUT_ROOT_FOLDER_NOLATEX}
#zip -r -9 ${OUT_ROOT_FOLDER}.zip ${OUT_ROOT_FOLDER}
#zip -r -9 ${OUT_ROOT_FOLDER_NOLATEX}.zip ${OUT_ROOT_FOLDER_NOLATEX}
