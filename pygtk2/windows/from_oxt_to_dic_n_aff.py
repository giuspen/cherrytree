#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../pygtkspellcheck/gtkspellcheck/'))
import oxt_extract

CURR_DIR = os.getcwd()

for dirpath, dirnames, filenames in os.walk(CURR_DIR):
    for filename in filenames:
        if dirpath == CURR_DIR:
            nation, ext = os.path.splitext(filename)
            if ext != ".oxt": continue
            oxt_extract.extract(filename, CURR_DIR, override=True)
