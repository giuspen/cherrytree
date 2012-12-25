#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import os, re, argparse, glob, gtk


parser = argparse.ArgumentParser()
parser.add_argument("source_file", help="source svg file")
parser.add_argument("dest_dir", help="destination directory")
args = parser.parse_args()

pixbuf = gtk.gdk.pixbuf_new_from_file(args.source_file)
new_pixbuf = pixbuf.scale_simple(48, 48, gtk.gdk.INTERP_HYPER)
new_pixbuf.save(os.path.join(args.dest_dir, args.source_file[:-3] + "png") , "png")
