#!/bin/bash
# based on https://gitlab.gnome.org/GNOME/gedit/-/tree/master/build-aux%2Fwin32
# and in particular make-gedit-installer.sh

GIT_CT_FOLDER="/home/${USER}/git/cherrytree"
GIT_CT_EXE="${GIT_CT_FOLDER}/future/cherrytree.exe"
GIT_CT_LICENSE="${GIT_CT_FOLDER}/license.txt"

NEW_MSYS2_FOLDER="C:/Users/${USER}/Desktop/CherryTree-msys2"
NEW_ROOT_FOLDER="C:/Users/${USER}/Desktop/CherryTree-root"
NEW_MINGW64_FOLDER="${NEW_ROOT_FOLDER}/mingw64"
NEW_ETC_GTK_FOLDER="${NEW_ROOT_FOLDER}/etc/gtk-3.0"
NEW_ETC_GTK_SETTINGS_INI="${NEW_ETC_GTK_FOLDER}/settings.ini"


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
pacman -Syu --root "${NEW_MSYS2_FOLDER}"
pacman -S --noconfirm --root "${NEW_MSYS2_FOLDER}" \
  filesystem \
  bash \
  pacman \
  mingw-w64-x86_64-gtkmm3 \
  mingw-w64-x86_64-gtksourceviewmm3 \
  mingw-w64-x86_64-libxml++2.6 \
  mingw-w64-x86_64-sqlite3
_result=$?
if [ "$_result" -ne "0" ]; then
  echo "failed to create base data via command 'pacman -S <packages names list> --noconfirm --root ${NEW_MSYS2_FOLDER}'"
  exit 1
fi
popd > /dev/null


echo "moving over necessary files from ${NEW_MSYS2_FOLDER} to ${NEW_ROOT_FOLDER}..."
mkdir -p ${NEW_ROOT_FOLDER}
mv -v ${NEW_MSYS2_FOLDER}/mingw64 ${NEW_ROOT_FOLDER}/


echo "removing unnecessary files..."
# remove .a files not needed for the installer
find ${NEW_MINGW64_FOLDER} -name "*.a" -exec rm -f {} \;
# remove unneeded binaries
find ${NEW_MINGW64_FOLDER} -not -name "g*.exe" -name "*.exe" -exec rm -f {} \;
rm -rf ${NEW_MINGW64_FOLDER}/bin/2to3*
rm -rf ${NEW_MINGW64_FOLDER}/bin/gdbm*.exe
rm -rf ${NEW_MINGW64_FOLDER}/bin/py*
rm -rf ${NEW_MINGW64_FOLDER}/bin/*-config
# remove other useless folders
rm -rf ${NEW_MINGW64_FOLDER}/var
rm -rf ${NEW_MINGW64_FOLDER}/ssl
rm -rf ${NEW_MINGW64_FOLDER}/include
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
# on windows we show the online help
rm -rf ${NEW_MINGW64_FOLDER}/share/help
rm -rf ${NEW_MINGW64_FOLDER}/share/gtk-doc
rm -rf ${NEW_MINGW64_FOLDER}/share/doc
# remove on the lib folder
rm -rf ${NEW_MINGW64_FOLDER}/lib/terminfo
rm -rf ${NEW_MINGW64_FOLDER}/lib/python2*
rm -rf ${NEW_MINGW64_FOLDER}/lib/pkgconfig
rm -rf ${NEW_MINGW64_FOLDER}/lib/peas-demo

# strip the binaries to reduce the size
find ${NEW_MINGW64_FOLDER} -name *.dll | xargs strip
find ${NEW_MINGW64_FOLDER} -name *.exe | xargs strip

# remove some translation which seem to add a lot of size
find ${NEW_MINGW64_FOLDER}/share/locale/ -type f | grep -v atk10.mo | grep -v libpeas.mo | grep -v gsettings-desktop-schemas.mo | grep -v json-glib-1.0.mo | grep -v glib20.mo | grep -v gedit.mo | grep -v gedit-plugins.mo | grep -v gdk-pixbuf.mo | grep -v gtk30.mo | grep -v gtk30-properties.mo | grep -v gtksourceview-4.mo | grep -v iso_*.mo | xargs rm
find ${NEW_MINGW64_FOLDER}/share/locale -type d | xargs rmdir -p --ignore-fail-on-non-empty


echo "set use native windows theme..."
[ -d ${NEW_ETC_GTK_FOLDER} ] || mkdir -p ${NEW_ETC_GTK_FOLDER}
echo "[Settings]" > ${NEW_ETC_GTK_SETTINGS_INI}
echo "gtk-theme-name=win32" >> ${NEW_ETC_GTK_SETTINGS_INI}


echo "copying cherrytree files..."
cp -v ${GIT_CT_EXE} "${NEW_ROOT_FOLDER}/mingw64/bin/"
cp -v ${GIT_CT_LICENSE} "${NEW_ROOT_FOLDER}/"
