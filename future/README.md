# Installation Guide

- [Debian/Linux Mint/Ubuntu](#building-cherrytree-on-ubuntu-2004)
- [Arch Linux/Manjaro Linux](#building-cherrytree-on-arch)
- [Fedora](#building-cherrytree-on-fedora)
- [MacOs](#building-cherrytree-on-macos)
- [Windows](#building-cherrytree-on-windows)


## Links on used libraries

https://www.gtkmm.org/en/documentation.shtml
https://developer.gnome.org/gtkmm-tutorial/stable/
https://developer.gnome.org/gtkmm/stable/
https://developer.gnome.org/gtkmm/stable/hierarchy.html

https://developer.gnome.org/gtksourceviewmm/stable/

https://developer.gnome.org/libxml++-tutorial/stable/
https://developer.gnome.org/libxml++/stable/

https://wiki.gnome.org/Projects/gspell
https://developer.gnome.org/gspell/stable/

https://cpputest.github.io/


## Build/Debug with Visual Studio Code on Linux

https://code.visualstudio.com/docs/setup/linux
required installation of Extension "C/C++"
```sh
cd cherrytree/future
code .
```
Build with: Ctrl+Shift+B
Debug with: F5

## To test install locally and create a package
```sh
cmake -DCMAKE_INSTALL_PREFIX=./local_usr ../future
make -j$(nproc --all)  && make install
cpack -G DEB
```

## Building Cherrytree on Ubuntu 20.04

Install dependencies::
```sh
sudo apt install build-essential libxml2-utils cmake libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libxml++2.6-dev libsqlite3-dev libcpputest-dev gettext python3-lxml libgspell-1-dev libcurl4-openssl-dev
```
Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
mkdir cherrytree/build
cd cherrytree/build
cmake ../future
make -j$(nproc --all)
./build/cherrytree
```
Install documentation:
```sh
sudo apt install devhelp libgtkmm-3.0-doc libgtksourceviewmm-3.0-doc libglibmm-2.4-doc libpangomm-1.4-doc libxml++2.6-doc libgspell-1-doc
```
devhelp
```sh
xdg-open /usr/share/doc/libgtkmm-3.0-doc/reference/html/index.html
xdg-open /usr/share/doc/libgtksourceviewmm-3.0-doc/reference/html/index.html
xdg-open /usr/share/doc/libglibmm-2.4-doc/reference/html/index.html
xdg-open /usr/share/doc/libpangomm-1.4-doc/reference/html/index.html
xdg-open /usr/share/doc/libxml++2.6-doc/reference/html/index.html
xdg-open /usr/share/doc/libgspell-1-dev/html/index.html
```

## Building Cherrytree on Arch

Install dependencies:
```sh
sudo pacman -S gtksourceviewmm libxml++2.6 python-lxml gspell
sudo pamac build cpputest
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
mkdir cherrytree/build
cd cherrytree/build
cmake ../future
make -j$(nproc --all)
./build/cherrytree
```

## Building Cherrytree on Fedora

Installd dependencies:
```sh
sudo dnf install @development-tools gcc-c++ libtool autoconf gtkmm30-devel gtksourceviewmm3-devel libxml++-devel libsq3-devel gettext-devel gettext intltool python3-lxml libxml2 gspell-devel
```

Install CppUTest:
```sh
git clone git://github.com/cpputest/cpputest.git
cd cpputest/cpputest_build
autoreconf .. -i
../configure
sudo make install
```

(OPTIONAL) See https://cpputest.github.io/ for more information on CppUTest

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
mkdir cherrytree/build
cd cherrytree/build
cmake ../future
make -j$(nproc --all)
./build/cherrytree
```

(OPTIONAL) Download Documentation
```sh
sudo dnf install gtkmm30-doc gtksourceviewmm3-doc glibmm24-doc glibmm24-doc libxml++-doc
```

(OPTIONAL) Open Documentation
```sh
xdg-open /usr/share/doc/gtkmm-3.0/reference/html/index.html
xdg-open /usr/share/doc/gtksourceviewmm-3.0/reference/html/index.html
xdg-open /usr/share/doc/glibmm-2.4/reference/html/index.html
xdg-open /usr/share/doc/pangomm-1.4/reference/html/index.html
xdg-open /usr/share/doc/libxml++2.6/reference/html/index.html
```

## Building Cherrytree on MacOS

Install dependencies:
```sh
brew install python3 cmake pkg-config gtksourceviewmm3 gnome-icon-theme gspell libxml++ cpputest
pip3 install lxml
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
mkdir cherrytree/build
cd cherrytree/build
cmake ../future
make -j$(sysctl -n hw.ncpu)
./build/cherrytree
```

To install:
```sh
make install
```


## Building Cherrytree on Windows

Install MSYS2: https://www.msys2.org/ (we cover here the packages for 64 bit installation)

Run the following command multiple times in the 'MSYS2 MinGW **64-bit**' terminal until there are no more updates:
```sh
pacman -Syuu
```

Install required packages to build cherrytree:
```sh
# toolchain and cmake
pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
# gtkmm3, libxml++, sqlite3
pacman -S --needed --noconfirm mingw-w64-x86_64-gtkmm3 mingw-w64-x86_64-gtksourceviewmm3 mingw-w64-x86_64-libxml++2.6 mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-gspell mingw-w64-x86_64-curl
# python3-lxml, gettext
pacman -S --needed --noconfirm mingw-w64-x86_64-python3-lxml mingw-w64-x86_64-gettext
# other
pacman -S --needed --noconfirm tar nano git mingw-w64-x86_64-meld3
# cpputest (missing package, we need to build manually)
wget https://github.com/cpputest/cpputest/releases/download/v3.8/cpputest-3.8.tar.gz
tar xf cpputest-3.8.tar.gz
cd cpputest-3.8
./autogen.sh
./configure --disable-memory-leak-detection
make
make install
```

use native windows theme
```sh
mkdir /etc/gtk-3.0
nano /etc/gtk-3.0/settings.ini
```
```ini
[Settings]
gtk-theme-name=win32
```
console settings
```sh
nano ~/.bashrc
```
```sh
export LC_ALL=C
CHERRYTREE_CONFIG_FOLDER="C:/Users/${USER}/AppData/Local/cherrytree"
[ -d ${CHERRYTREE_CONFIG_FOLDER} ] || mkdir -p ${CHERRYTREE_CONFIG_FOLDER}
alias l="ls -lah --color"
bind '"\e[A":history-search-backward'
bind '"\e[B":history-search-forward'
```
```sh
# build Release (optimised)
./build.sh Release
# build Debug (not optimised, with debug symbols)
./build.sh Debug
# run cherrytree
./build/cherrytree.exe
# run unit tests
./build/tests/run_tests.exe
```
