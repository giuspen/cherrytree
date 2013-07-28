#!/bin/sh

cd ..

intltool-extract --type=gettext/glade glade/cherrytree.glade

xgettext --language=Python --from-code=utf-8 --keyword=_ --keyword=N_ --output=locale/cherrytree.pot modules/support.py modules/findreplace.py modules/core.py modules/tablez.py modules/codeboxes.py modules/cons.py modules/imports.py modules/pgsc_spellcheck.py glade/cherrytree.glade.h
