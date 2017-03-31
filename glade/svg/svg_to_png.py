#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

from gi.repository import Gtk
from gi.repository import GdkPixbuf
import os
import re
import argparse
import glob


parser = argparse.ArgumentParser()
parser.add_argument("source_file", help="source svg file")
parser.add_argument("dest_dir", help="destination directory")
args = parser.parse_args()

pixbuf = GdkPixbuf.Pixbuf.new_from_file(args.source_file)
new_pixbuf = pixbuf.scale_simple(48, 48, GdkPixbuf.InterpType.HYPER)
new_pixbuf.save(os.path.join(args.dest_dir, args.source_file[:-3] + "png") , "png")
