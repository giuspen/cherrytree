# CherryTree
A hierarchical note taking application, featuring rich text and syntax highlighting, storing data in a single XML or SQLite file.
The project home page is [giuspen.com/cherrytree](https://www.giuspen.com/cherrytree/).

# Installation Guide

- [Debian/Linux Mint/Ubuntu](#building-cherrytree-on-ubuntu-2004)
- [Arch Linux/Manjaro Linux](#building-cherrytree-on-arch)
- [Fedora](#building-cherrytree-on-fedora)
- [Opensuse](#building-cherrytree-on-opensuse)
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


## Build/Debug with Visual Studio Code on Linux

https://code.visualstudio.com/docs/setup/linux
required installation of Extension "C/C++"
```sh
cd cherrytree
code .
```
Build with: Ctrl+Shift+B
Debug with: F5

## Build/Debug with Visual Studio Code using a container

It is possible to use a container as a full-featured development environment from VS Code.
This works on any operating system that supports Docker.

1. Install the [system requirements](https://code.visualstudio.com/docs/remote/containers#_system-requirements).
2. Open the project in VS Code.
3. (optional) Edit `.devcontainer/devcontainer.json` to set your `DISPLAY` environment variable and/or edit other settings.
   It is also possible to run the container on a remote Docker host, see the comment at the end.
4. Run the *Remote-Containers: Open Folder in Container...* command.
5. See previous section for Build and Debug instructions.

## To test install locally and create a package
```sh
cmake -DCMAKE_INSTALL_PREFIX=./local_usr ../
make -j$(nproc --all)  && make install
cpack -G DEB
```

## Building Cherrytree on Ubuntu 20.04

Install dependencies::
```sh
sudo apt install build-essential libxml2-utils cmake libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libxml++2.6-dev libsqlite3-dev gettext libgspell-1-dev libcurl4-openssl-dev libuchardet-dev
```
Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
git submodule update --init
mkdir cherrytree/build
cd cherrytree/build
cmake ../
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
sudo pacman -S gtksourceviewmm libxml++2.6 gspell
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
git submodule update --init
mkdir cherrytree/build
cd cherrytree/build
cmake ../
make -j$(nproc --all)
./build/cherrytree
```

## Building Cherrytree on Fedora

Install dependencies:
```sh
sudo dnf install @development-tools gcc-c++ libtool autoconf gtkmm30-devel gtksourceviewmm3-devel libxml++-devel libsq3-devel gettext-devel gettext intltool libxml2 gspell-devel
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
git submodule update --init
mkdir cherrytree/build
cd cherrytree/build
cmake ../
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

## Building Cherrytree on Opensuse

Install dependencies:
```sh
sudo zypper install cmake gcc-c++ gtkmm3-devel gtksourceviewmm3_0-devel gspell-devel libxml++26-devel sqlite3-devel libcurl-devel libuchardet-devel
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
git submodule update --init
mkdir cherrytree/build
cd cherrytree/build
cmake ../
make -j$(nproc --all)
./build/cherrytree
```

## Building Cherrytree on MacOS

Install dependencies:
```sh
brew install python3 cmake pkg-config gtksourceviewmm3 gnome-icon-theme gspell libxml++ curl uchardet
```

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
cd cherrytree
git submodule update --init
mkdir build
cd build
cmake ../
make -j$(sysctl -n hw.ncpu)
./build/cherrytree
```

To install:
```sh
make install
```


## Building Cherrytree on Windows

Install MSYS2: https://www.msys2.org/ (we cover here the packages for 64 bit installation)

Launch 'MSYS2 MinGW 64-bit' terminal (there are 3 different terminals, make sure it is 64-bit otherwise it will cause issues)

Run the following command multiple times there until there are no more updates:
```sh
pacman -Syuu
```

Install required packages to build cherrytree:
```sh
# toolchain and cmake
pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
# gtkmm3, gtksourceviewmm3, libxml++2.6, sqlite3, gspell, curl
pacman -S --needed --noconfirm mingw-w64-x86_64-gtkmm3 mingw-w64-x86_64-gtksourceviewmm3 mingw-w64-x86_64-libxml++2.6 mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-gspell mingw-w64-x86_64-curl mingw-w64-x86_64-uchardet
# gettext, git, nano, meld3
pacman -S --needed --noconfirm mingw-w64-x86_64-gettext git nano mingw-w64-x86_64-meld3
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

Get cherrytree source, compile and run:
```sh
git clone https://github.com/giuspen/cherrytree.git
git submodule update --init
cd cherrytree

# build Release (optimised)
./build.sh Release
# build Debug (not optimised, with debug symbols)
./build.sh Debug
# run cherrytree
./build/cherrytree.exe
```

Troubleshooting:
- Cannot build: make sure to start 64-bit terminal
- Cannot build: remove `cherrytree/build` folder and start `build.sh` script again
- Cannot start cherrytree: you either have to run cherrytree from the msys2 mingw64 terminal or copy and replace cherrytree in `cherrytree_0.99.X_win64_portable` folder (downloaded from the site) by the new one, so dependencies are fulfilled 
