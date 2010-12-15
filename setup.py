#!/usr/bin/env python

# for linux install: "python setup.py install --prefix=/usr --exec-prefix=/usr -f"
# for windows exe creation: "python setup.py py2exe"

from distutils.core import setup

try: import py2exe
except: pass

import os, glob, sys, subprocess
import __builtin__
def _(transl_str):
   return transl_str
__builtin__._ = _
sys.path.append(os.path.join(os.getcwd(), "modules"))
import cons

if "py2exe" in sys.argv:
   data_files = [("glade", glob.glob("glade/*.*") )]
   for lang in cons.AVAILABLE_LANGS:
      if lang in ["default", "en"]: continue
      data_files.append( ("share/locale/%s/LC_MESSAGES" % lang, ["locale/%s/LC_MESSAGES/cherrytree.mo" % lang] ) )
else:
   data_files = [
                  ("share/icons/hicolor/scalable/apps", ["linux/cherrytree.svg"] ),
                  ("share/applications", ["linux/cherrytree.desktop"] ),
                  ("share/cherrytree/glade", glob.glob("glade/*.*") ),
                  ("share/cherrytree/modules", glob.glob("modules/*.py") ),
                  ("share/mime/packages", ["linux/cherrytree.xml"]),
                  ("share/mime-info", ["linux/cherrytree.mime", "linux/cherrytree.keys"]),
                  ("share/application-registry", ["linux/cherrytree.applications"])
               ]
   for lang in cons.AVAILABLE_LANGS:
      if lang in ["default", "en"]: continue
      data_files.append( ("share/locale/%s/LC_MESSAGES" % lang, ["locale/%s/LC_MESSAGES/cherrytree.mo" % lang] ) )

setup(
   name = "CherryTree",
   description = "Hierarchical Note Taking",
   long_description = "A Hierarchical Note Taking Application, featuring Rich Text and Syntax Highlighting",
   version = cons.VERSION,
   author = "Giuseppe Penone",
   author_email = "giuspen@gmail.com",
   url = "http://www.giuspen.com/cherrytree/",
   license = "GPL",
   scripts = ["cherrytree"],
   windows = [{"script": "cherrytree",
               "icon_resources": [(1, "glade/cherrytree.ico")]
               }],
   options={"py2exe": {
               "includes": "pango,cairo,pangocairo,atk,gobject,gtk,gtksourceview2,gio",
               "dll_excludes": [
                             "iconv.dll","intl.dll","libatk-1.0-0.dll",
                             "libgdk_pixbuf-2.0-0.dll","libgdk-win32-2.0-0.dll",
                             "libglib-2.0-0.dll","libgmodule-2.0-0.dll",
                             "libgobject-2.0-0.dll","libgthread-2.0-0.dll",
                             "libgtk-win32-2.0-0.dll","libpango-1.0-0.dll",
                             "libpangowin32-1.0-0.dll"],
                      }
            },
   data_files = data_files
)

if "py2exe" in sys.argv: print "remember to move dist/share/locale to dist/locale and copy C:\gtksourceview\share\gtksourceview-2.0 to dist/share/"
else: subprocess.call("update-desktop-database")
