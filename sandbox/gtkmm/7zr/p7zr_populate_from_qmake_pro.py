#!/usr/bin/env python2
# -*- coding: UTF-8 -*-

import os
import shutil
import argparse
import re
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SRC_DIR = os.path.join(SCRIPT_DIR, "src")
MAKEFILE_AM_TEMPLATE = os.path.join(SCRIPT_DIR, "template_Makefile.am")
MAKEFILE_AM_TARGET = os.path.join(SCRIPT_DIR, "Makefile.am")
CONFIGURE_AC_TEMPLATE = os.path.join(SCRIPT_DIR, "template_configure.ac")
CONFIGURE_AC_TARGET = os.path.join(SCRIPT_DIR, "configure.ac")


class P7ZRAutotools(object):


    def __init__(self, p7zip_sources, dry_run):
        self._p7zip_sources = p7zip_sources
        self._dry_run = dry_run

        if self._p7zip_sources[-1] in ("/", "\\"):
            self._p7zip_sources = self._p7zip_sources[:-1]
        self._version = os.path.basename(self._p7zip_sources).split("_")[-1]
        self._all_includedirs_from = []
        self._all_sourcefiles_to = []
        self._all_headerfiles_to = []


    def _get_pro_paths(self, qmake_pro_dirpath, qmake_pro_filepath, starting_token):

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


    def _source_parse_headers(self, filepath):

        with open(filepath, "r") as fd:
            txt_content = fd.read()
        matches = re.findall(r'#include\s+\"([^\"]+)\"', txt_content, re.MULTILINE)
        for head_rel_path in matches:
            #print headpath
            this_dirpath = os.path.dirname(filepath)
            for curr_dirpath in ([this_dirpath] + self._all_includedirs_from):
                headpath = os.path.realpath(os.path.join(curr_dirpath, head_rel_path))
                if os.path.isfile(headpath):
                    self._filepath_to_copy(headpath)
                    break


    def _filepath_do_copy(self, filepath, target_filepath):

        if self._dry_run is True:
            print "{} -> {}".format(filepath, target_filepath)
        else:
            target_dirpath = os.path.dirname(target_filepath)
            if not os.path.isdir(target_dirpath):
                os.makedirs(target_dirpath)
            shutil.copyfile(filepath, target_filepath)


    def _filepath_to_copy(self, filepath):

        rel_filepath = os.path.relpath(filepath, self._p7zip_sources)
        target_filepath = os.path.join(SRC_DIR, rel_filepath)

        parse_for_includes = False
        if self._is_header(filepath):
            if target_filepath not in self._all_headerfiles_to:
                self._all_headerfiles_to.append(target_filepath)
                self._filepath_do_copy(filepath, target_filepath)
                parse_for_includes = True
        else:
            if target_filepath not in self._all_sourcefiles_to:
                self._all_sourcefiles_to.append(target_filepath)
                self._filepath_do_copy(filepath, target_filepath)
                parse_for_includes = True

        if parse_for_includes is True:
            self._source_parse_headers(filepath)


    def _is_header(self, filepath):

        return filepath.rsplit(".", 2)[-1] in ("h", "H")


    def generate(self):

        qmake_pro_dirpath = os.path.join(self._p7zip_sources, "CPP", "7zip", "QMAKE", "7zr")
        qmake_pro_filepath = os.path.join(qmake_pro_dirpath, "7zr.pro")

        for dirpath in self._get_pro_paths(qmake_pro_dirpath, qmake_pro_filepath, "INCLUDEPATH ="):
            self._all_includedirs_from.append(dirpath)
            #print dirpath

        for filepath in self._get_pro_paths(qmake_pro_dirpath, qmake_pro_filepath, "SOURCES +="):
            self._filepath_to_copy(filepath)

        with open(qmake_pro_filepath, "r") as fd:
            qmake_pro_txt = fd.read()
        all_defines_list = re.findall(r"DEFINES\s*\+=\s*([^\s]+)", qmake_pro_txt, re.MULTILINE)

        all_defines_txt = ""
        last_index = len(all_defines_list)-1
        for i, d in enumerate(all_defines_list):
            new_element = "    -D"+d
            if i < last_index:
                new_element += " \\\n"
            all_defines_txt += new_element

        all_sources_txt = ""
        all_sources_list = self._all_sourcefiles_to + self._all_headerfiles_to
        last_index = len(all_sources_list)-1
        for i, s in enumerate(all_sources_list):
            new_element = "    "+os.path.relpath(s, SCRIPT_DIR)
            if i < last_index:
                new_element += " \\\n"
            all_sources_txt += new_element

        if self._dry_run is True:
            print self._version
            print all_defines_txt
            print all_sources_txt
        else:
            with open(CONFIGURE_AC_TEMPLATE, "r") as fd:
                configure_ac_txt = fd.read()
            configure_ac_txt = configure_ac_txt.replace("__VERSION_HERE__", self._version)
            with open(CONFIGURE_AC_TARGET, "w") as fd:
                fd.write(configure_ac_txt)

            with open(MAKEFILE_AM_TEMPLATE, "r") as fd:
                makefile_am_txt = fd.read()
            makefile_am_txt = makefile_am_txt.replace("__DEFINES_HERE__", all_defines_txt)
            makefile_am_txt = makefile_am_txt.replace("__SOURCE_FILES_HERE__", all_sources_txt)
            with open(MAKEFILE_AM_TARGET, "w") as fd:
                fd.write(makefile_am_txt)

        for filename in ("NEWS", "README", "AUTHORS", "ChangeLog"):
            subprocess.call(("touch", os.path.join(SCRIPT_DIR, filename)))
        po_dir = os.path.join(SCRIPT_DIR, "po")
        if not os.path.isdir(po_dir):
            os.mkdir(po_dir)


if "__main__" == __name__:
    parser = argparse.ArgumentParser()
    parser.add_argument('p7zip_sources', nargs='?', default="/home/giuspen/Software/p7zip_16.02", help='p7zip sources dirpath')
    parser.add_argument("-d", "--dry_run", action="store_true", help="just print, do not take any action")
    args = parser.parse_args()
    p7zr_autotools = P7ZRAutotools(args.p7zip_sources, args.dry_run)
    p7zr_autotools.generate()
