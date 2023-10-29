# CherryTree
A hierarchical note taking application, featuring rich text and syntax highlighting, storing data in a single XML or SQLite file.
The project home page is [giuspen.net/cherrytree](https://www.giuspen.net/cherrytree/).

## Getting Started
### Prerequisites
The current (python) version requires:
* python2
* python-gtk2
* python-gtksourceview2
* p7zip-full
* python-dbus
* python-enchant
* python-chardet

Then, after cloning, run in the top folder `./cherrytree` or `python2 cherrytree`.

NOTE: The dictionaries for the spellcheck have packages names like hunspell-it, hunspell-fr, ...

### Installation
Look at [giuspen.net/cherrytree/#downl](https://www.giuspen.net/cherrytree/#downl) for available installers.

### Development
The development is currently happening on the **master** branch in the root folder being a C++/GTKmm porting.

The latest stable python release is also from the master branch.

The branch **pygi** is a discontinued attempt to port to the python bindings for GTK3. Reason for the failure in the issue [python-gtksourceview2 deprecated, please port to PyGI](https://github.com/giuspen/cherrytree/issues/125).
