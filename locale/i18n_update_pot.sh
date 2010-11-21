#!/bin/sh

cd ..

intltool-extract --type=gettext/glade glade/cherrytree.glade

xgettext --language=Python --keyword=_ --keyword=N_ --output=locale/cherrytree.pot modules/support.py modules/findreplace.py modules/core.py modules/tablez.py modules/codeboxes.py modules/cons.py glade/cherrytree.glade.h
