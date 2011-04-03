#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
#       main.py
#       
#       Copyright 2009-2011 Giuseppe Penone <giuspen@gmail.com>
#       
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import sys, os, gtk, gettext
import cons, core


def main(OPEN_WITH_FILE):
   """Everything Starts from Here"""
   if sys.platform[0:3] == "win":
      import warnings
      warnings.filterwarnings("ignore")
   else:
      try:
         # change process name
         import ctypes, ctypes.util
         libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("libc"))
         libc.prctl(15, cons.APP_NAME, 0, 0, 0)
      except: print "libc.prctl not available, the process name will be python and not cherrytree"
   try:
      # change locale text domain
      import locale
      locale.bindtextdomain(cons.APP_NAME, cons.LOCALE_PATH)
   except: print "locale.bindtextdomain not available, the glade i18n may not work properly"
   # language installation
   if os.path.isfile(cons.LANG_PATH):
      lang_file_descriptor = file(cons.LANG_PATH, 'r')
      lang_str = lang_file_descriptor.read()
      lang_file_descriptor.close()
      if lang_str != 'default': os.environ["LANGUAGE"] = lang_str
   else: lang_str = 'default'
   try: gettext.translation(cons.APP_NAME, cons.LOCALE_PATH).install()
   except:
      import __builtin__
      def _(transl_str):
         return transl_str
      __builtin__._ = _
   # start the app
   core.CherryTree(lang_str, True, OPEN_WITH_FILE)
   gtk.main() # start the gtk main loop
   return 0
