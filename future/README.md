
# https://www.gtkmm.org/en/documentation.shtml
# https://developer.gnome.org/gtkmm-tutorial/stable/
# https://developer.gnome.org/gtkmm/stable/
# https://developer.gnome.org/gtkmm/stable/hierarchy.html

# https://developer.gnome.org/gtksourceviewmm/stable/

# https://developer.gnome.org/libxml++-tutorial/stable/
# https://developer.gnome.org/libxml++/stable/

# https://wiki.gnome.org/Projects/gspell
# https://developer.gnome.org/gspell/stable/

# https://cpputest.github.io/


# ---- Build/Debug with Visual Studio Code on Linux ----

# https://code.visualstudio.com/docs/setup/linux
# required installation of Extension "C/C++"
cd cherrytree/future
code .
# Build with: Ctrl+Shift+B
# Debug with: F5


# ---- Building Cherrytree in Ubuntu ----

# Install Dependencies
sudo apt install build-essential libtool autoconf libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libxml++2.6-dev libsqlite3-dev libcpputest-dev autopoint gettext intltool python3-lxml libxml2-utils libgspell-1-dev

# build Debug
./build.sh
# or build Release
./build.sh R
./cherrytree


# packages for documentation
sudo apt install devhelp libgtkmm-3.0-doc libgtksourceviewmm-3.0-doc libglibmm-2.4-doc libpangomm-1.4-doc libxml++2.6-doc libgspell-1-doc

# =>
devhelp
xdg-open /usr/share/doc/libgtkmm-3.0-doc/reference/html/index.html
xdg-open /usr/share/doc/libgtksourceviewmm-3.0-doc/reference/html/index.html
xdg-open /usr/share/doc/libglibmm-2.4-doc/reference/html/index.html
xdg-open /usr/share/doc/libpangomm-1.4-doc/reference/html/index.html
xdg-open /usr/share/doc/libxml++2.6-doc/reference/html/index.html
xdg-open /usr/share/doc/libgspell-1-dev/html/index.html


# ---- Building Cherrytree in Arch ----

# Install Dependencies
sudo pacman -S gtksourceviewmm libxml++2.6 python-lxml gspell
sudo pamac build cpputest


# ---- Building Cherrytree in Windows ----

# install MSYS2: https://www.msys2.org/ (we cover here the packages for 64 bit installation)

# run the following command multiple times in the 'MSYS2 MinGW 64-bit' terminal until there are no more updates:
pacman -Syuu

# install required packages to build cherrytree:
# toolchain
pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain
# make and autotools
pacman -S --needed --noconfirm make pkgconfig autoconf automake automake-wrapper libtool intltool gettext-devel
# gtkmm3, libxml++, sqlite3
pacman -S --needed --noconfirm mingw-w64-x86_64-gtkmm3 mingw-w64-x86_64-gtksourceviewmm3 mingw-w64-x86_64-libxml++2.6 mingw-w64-x86_64-sqlite3 mingw-w64-gspell
# python3-lxml
pacman -S --needed --noconfirm mingw-w64-x86_64-python3-lxml
# other
pacman -S --needed --noconfirm perl tar nano p7zip git mingw-w64-x86_64-meld3
# cpputest (missing package, we need to build manually)
wget https://github.com/cpputest/cpputest/releases/download/v3.8/cpputest-3.8.tar.gz
tar xf cpputest-3.8.tar.gz
cd cpputest-3.8
./autogen.sh
./configure --disable-memory-leak-detection
make
make install
# use native windows theme
mkdir /etc/gtk-3.0
nano /etc/gtk-3.0/settings.ini
####################
[Settings]
gtk-theme-name=win32
####################
# console settings
nano ~/.bashrc
####################
export LC_ALL=C
CHERRYTREE_CONFIG_FOLDER="C:/Users/${USER}/AppData/Local/cherrytree"
[ -d ${CHERRYTREE_CONFIG_FOLDER} ] || mkdir -p ${CHERRYTREE_CONFIG_FOLDER}
alias l="ls -lah --color"
bind '"\e[A":history-search-backward'
bind '"\e[B":history-search-forward'
####################

# build and run cherrytree
# build Debug
./build.sh
# or build Release
./build.sh R
./cherrytree.exe

## NOTES: *** FIND DEPENDENCIES *** of cherrytree.exe and copy to ~/dependencies/
if [ ! -d ~/dependencies ]; then mkdir ~/dependencies; fi
cp `ldd cherrytree.exe | grep /mingw64/bin | awk '{print $3}'` ~/dependencies/

## NOTES: *** DO NOT SHOW CONSOLE WINDOW *** in Windows OS
cherrytree_LDFLAGS = -mwindows


# ---- Building Cherrytree in Fedora ----

# Install Dependencies
# ====================
sudo dnf install @development-tools gcc-c++ libtool autoconf gtkmm30-devel gtksourceviewmm3-devel libxml++-devel libsq3-devel gettext-devel gettext intltool python3-lxml libxml2 gspell-devel


# Install CppUTest
# ================
git clone git://github.com/cpputest/cpputest.git
cd cpputest/cpputest_build
autoreconf .. -i
../configure
sudo make install

# (OPTIONAL) See https://cpputest.github.io/ for more information on CppUTest


# Build Cherrytree
# ================
git clone https://github.com/giuspen/cherrytree.git
cd cherrytree/future

# build Debug (See #troubleshooting below if you encounter errors.)
./build.sh
# or build Release
./build.sh R
./cherrytree


# Troubleshooting

# If autogen.sh fails because cpputest is not found, set PKG_CONFIG_PATH and try running autogen again.
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

# If cpputest is still not found, reboot, set PKG_CONFIG_PATH again, run autogen.sh again.


# (OPTIONAL) Download Documentation
===================================
sudo dnf install gtkmm30-doc gtksourceviewmm3-doc glibmm24-doc glibmm24-doc libxml++-doc


# (OPTIONAL) Open Documentation
===============================
xdg-open /usr/share/doc/gtkmm-3.0/reference/html/index.html
xdg-open /usr/share/doc/gtksourceviewmm-3.0/reference/html/index.html
xdg-open /usr/share/doc/glibmm-2.4/reference/html/index.html
xdg-open /usr/share/doc/pangomm-1.4/reference/html/index.html
xdg-open /usr/share/doc/libxml++2.6/reference/html/index.html
