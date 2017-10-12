#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import os
import shutil
import argparse

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SRC_DIR = os.path.join(SCRIPT_DIR, "src")
MAKEFILE_AM_TEMPLATE = os.path.join(SCRIPT_DIR, "template_Makefile.am")
MAKEFILE_AM_TARGET = os.path.join(SCRIPT_DIR, "Makefile.am")
CONFIGURE_AC_TEMPLATE = os.path.join(SCRIPT_DIR, "template_configure.ac")
CONFIGURE_AC_TARGET = os.path.join(SCRIPT_DIR, "configure.ac")


def get_pro_paths(qmake_pro_dirpath, qmake_pro_filepath, starting_token):

    with open(qmake_pro_filepath, "r") as fd:
        target_acquired = False
        for str_line in fd:
            str_line_clean = str_line.replace("\n", "").replace("\r", "")
            if target_acquired is False:
                if str_line_clean.startswith(starting_token):
                    target_acquired = True
            else:
                if not str_line_clean.endswith(" \\"):
                    break
                path = os.path.realpath(os.path.join(qmake_pro_dirpath, str_line[:-2].strip()))
                assert os.path.exists(path)
                yield path


def main(p7zip_dirpath, dry_run):

    version = os.path.basename(p7zip_dirpath).split("_")[-1]
    print version

    qmake_pro_dirpath = os.path.join(p7zip_dirpath, "CPP", "7zip", "QMAKE", "7zr")
    qmake_pro_filepath = os.path.join(qmake_pro_dirpath, "7zr.pro")
    all_includedirs_from = []
    all_sourcefiles_to = []
    all_headerfiles_to = []

    for dirpath in get_pro_paths(qmake_pro_dirpath, qmake_pro_filepath, "INCLUDEPATH ="):
        all_includedirs_from.append(dirpath)
        print dirpath

    for filepath in get_pro_paths(qmake_pro_dirpath, qmake_pro_filepath, "SOURCES +="):
        rel_filepath = os.path.relpath(filepath, p7zip_dirpath)
        target_filepath = os.path.join(SRC_DIR, rel_filepath)
        all_sourcefiles_to.append(target_filepath)
        if dry_run is True:
            print "{} -> {}".format(filepath, target_filepath)
        else:
            target_dirpath = os.path.dirname(target_filepath)
            if not os.path.isdir(target_dirpath):
                os.makedirs(target_dirpath)
            shutil.copyfile(filepath, target_filepath)


if "__main__" == __name__:
    parser = argparse.ArgumentParser()
    parser.add_argument('p7zip_sources', nargs='?', default="/home/giuspen/Software/p7zip_16.02", help='p7zip sources dirpath')
    parser.add_argument("-d", "--dry_run", action="store_true", help="just print, do not take any action")
    args = parser.parse_args()
    main(args.p7zip_sources, args.dry_run)
