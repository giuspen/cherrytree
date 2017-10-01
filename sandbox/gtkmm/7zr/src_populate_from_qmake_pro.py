#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import os
import shutil
import argparse

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SRC_DIR = os.path.join(SCRIPT_DIR, "src")


def f_main(p7zip_dirpath):
    qmake_pro_dirpath = os.path.join(p7zip_dirpath, "CPP", "7zip", "QMAKE", "7zr")
    qmake_pro_filepath = os.path.join(qmake_pro_dirpath, "7zr.pro")
    with open(qmake_pro_filepath, "r") as fd:
        sources_found = False
        for str_line in fd:
            str_line_clean = str_line.replace("\n", "").replace("\r", "")
            if sources_found is False:
                if str_line_clean.startswith("SOURCES +="):
                    sources_found = True
            else:
                if not str_line_clean.endswith(" \\"):
                    break
                filepath = os.path.realpath(os.path.join(qmake_pro_dirpath, str_line[:-2].strip()))
                assert os.path.isfile(filepath)
                rel_filepath = os.path.relpath(filepath, p7zip_dirpath)
                target_filepath = os.path.join(SRC_DIR, rel_filepath)
                target_dirpath = os.path.dirname(target_filepath)
                if not os.path.isdir(target_dirpath): os.makedirs(target_dirpath)
                shutil.copyfile(filepath, target_filepath)


parser = argparse.ArgumentParser()
parser.add_argument('p7zip_sources', nargs='?', default="/home/giuspen/Software/p7zip_16.02", help='p7zip sources dirpath')
args = parser.parse_args()
f_main(args.p7zip_sources)
