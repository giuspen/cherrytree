#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys, shutil, glob, builtins

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
MODULES_DIR = os.path.join(SCRIPT_DIR, "modules")
sys.path.insert(0, MODULES_DIR)

builtins.SHARE_PATH = SCRIPT_DIR
import cons

BLACKLIST = [".git", ".gitignore"]

DEST_DIR = os.path.join(os.path.dirname(SCRIPT_DIR), "cherrytree-"+cons.VERSION)
if len(sys.argv) > 1:
    DEST_DIR += "+r" + sys.argv[1]
#print DEST_DIR

if not os.path.isdir(DEST_DIR):
    os.mkdir(DEST_DIR)

for pycfilepath in glob.glob(os.path.join(MODULES_DIR, "*.pyc")):
    os.remove(pycfilepath)

for element in os.listdir(SCRIPT_DIR):
    if not element in BLACKLIST:
        src_abspath = os.path.join(SCRIPT_DIR, element)
        dst_abspath = os.path.join(DEST_DIR, element)
        if os.path.isdir(src_abspath):
            shutil.copytree(src_abspath, dst_abspath)
        else:
            shutil.copy2(src_abspath, dst_abspath)
