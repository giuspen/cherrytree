#!/bin/bash
# based on https://gitlab.gnome.org/GNOME/gedit/-/tree/master/build-aux%2Fwin32
# and in particular make-gedit-installer.sh

GIT_CT_FOLDER="/home/${USER}/git/cherrytree"
GIT_CT_EXE="${GIT_CT_FOLDER}/future/cherrytree.exe"
GIT_CT_LICENSE="${GIT_CT_FOLDER}/license.txt"

NEW_ROOT_FOLDER="C:/Users/${USER}/Desktop/CherryTree-Root"
NEW_ETC_GTK_FOLDER="${NEW_ROOT_FOLDER}/etc/gtk-3.0"
NEW_ETC_GTK_SETTINGS_INI="${NEW_ETC_GTK_FOLDER}/settings.ini"


echo "installing minimal filesystem to ${NEW_ROOT_FOLDER}..."
[ -d ${NEW_ROOT_FOLDER} ] && rm -rf ${NEW_ROOT_FOLDER}
mkdir -p "${NEW_ROOT_FOLDER}"
pushd "${NEW_ROOT_FOLDER}" > /dev/null

mkdir -p var/lib/pacman
mkdir -p var/log
mkdir -p tmp

pacman -Syu --root "${NEW_ROOT_FOLDER}"
pacman -S filesystem bash pacman mingw-w64-x86_64-gtkmm3 mingw-w64-x86_64-gtksourceviewmm3 mingw-w64-x86_64-libxml++2.6 mingw-w64-x86_64-sqlite3 --noconfirm --root "${NEW_ROOT_FOLDER}"
_result=$?
if [ "$_result" -ne "0" ]; then
  echo "failed to create base data via command 'pacman -S <packages names list> --noconfirm --root ${NEW_ROOT_FOLDER}'"
fi
popd > /dev/null


# remove .a files not needed for the installer
find ${NEW_ROOT_FOLDER}/mingw64 -name "*.a" -exec rm -f {} \;
# remove unneeded binaries
find ${NEW_ROOT_FOLDER}/mingw64 -not -name "g*.exe" -name "*.exe" -exec rm -f {} \;
rm -rf ${NEW_ROOT_FOLDER}/mingw64/bin/gdbm*.exe
rm -rf ${NEW_ROOT_FOLDER}/mingw64/bin/py*
rm -rf ${NEW_ROOT_FOLDER}/mingw64/bin/*-config
# remove other useless folders
rm -rf ${NEW_ROOT_FOLDER}/mingw64/var
rm -rf ${NEW_ROOT_FOLDER}/mingw64/ssl
rm -rf ${NEW_ROOT_FOLDER}/mingw64/include
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/man
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/readline
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/info
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/aclocal
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/gnome-common
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/glade
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/gettext
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/terminfo
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/tabset
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/pkgconfig
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/bash-completion
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/appdata
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/gdb
# on windows we show the online help
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/help
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/gtk-doc
rm -rf ${NEW_ROOT_FOLDER}/mingw64/share/doc
# remove on the lib folder
rm -rf ${NEW_ROOT_FOLDER}/mingw64/lib/terminfo
rm -rf ${NEW_ROOT_FOLDER}/mingw64/lib/python2*
rm -rf ${NEW_ROOT_FOLDER}/mingw64/lib/pkgconfig
rm -rf ${NEW_ROOT_FOLDER}/mingw64/lib/peas-demo

# strip the binaries to reduce the size
find ${NEW_ROOT_FOLDER}/mingw64 -name *.dll | xargs strip
find ${NEW_ROOT_FOLDER}/mingw64 -name *.exe | xargs strip

# remove some translation which seem to add a lot of size
find ${NEW_ROOT_FOLDER}/mingw64/share/locale/ -type f | grep -v atk10.mo | grep -v libpeas.mo | grep -v gsettings-desktop-schemas.mo | grep -v json-glib-1.0.mo | grep -v glib20.mo | grep -v gedit.mo | grep -v gedit-plugins.mo | grep -v gdk-pixbuf.mo | grep -v gtk30.mo | grep -v gtk30-properties.mo | grep -v gtksourceview-4.mo | grep -v iso_*.mo | xargs rm
find ${NEW_ROOT_FOLDER}/mingw64/share/locale -type d | xargs rmdir -p --ignore-fail-on-non-empty


echo "set use native windows theme..."
[ -d ${NEW_ETC_GTK_FOLDER} ] || mkdir -p ${NEW_ETC_GTK_FOLDER}
echo "[Settings]" > ${NEW_ETC_GTK_SETTINGS_INI}
echo "gtk-theme-name=win32" >> ${NEW_ETC_GTK_SETTINGS_INI}


echo "copying cherrytree files..."
cp -v ${GIT_CT_EXE} "${NEW_ROOT_FOLDER}/mingw64/bin/"
cp -v ${GIT_CT_LICENSE} "${NEW_ROOT_FOLDER}/"
