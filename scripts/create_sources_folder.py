#!/usr/bin/env python3

import os
import shutil
import builtins
import re

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEBIAN_CHANGELOG_PATH = os.path.join(ROOT_DIR, "debian", "changelog")
PO_DIR = os.path.join(ROOT_DIR, "po")
ICONS_CC_PATH = os.path.join(ROOT_DIR, "src", "ct", "icons.gresource.cc")
MANUAL_GZ_PATH = os.path.join(ROOT_DIR, "data", "cherrytree.1.gz")
DEBIAN_FILES_PATH = os.path.join(ROOT_DIR, "debian", "files")
BLACKLIST = (
    ".git",
    ".gitignore",
    ".gitmodules",
    ".travis.yml",
    "build",
    "config.h",
    "pygtk2",
    "flatpak",
    "snap",
    "supporters",
    "sandbox"
)
VERSION = "?"
with open(DEBIAN_CHANGELOG_PATH, "r") as fd:
    for fileline in fd:
        match = re.search(r"cherrytree +\(([0-9]+\.[0-9]+\.[0-9]+)[-+]", fileline)
        if match is not None:
            VERSION = match.group(1)
            #print(VERSION)
            break
DEST_DIR = os.path.join(os.path.dirname(ROOT_DIR), "cherrytree_"+VERSION)

for element in os.listdir(PO_DIR):
    abspath = os.path.join(PO_DIR, element)
    if os.path.isdir(abspath):
        print("rm {}".format(abspath))
        shutil.rmtree(abspath)

for filepath in (ICONS_CC_PATH, MANUAL_GZ_PATH, DEBIAN_FILES_PATH):
    if os.path.isfile(filepath):
        os.remove(filepath)

if not os.path.isdir(DEST_DIR):
    os.mkdir(DEST_DIR)

for element in os.listdir(ROOT_DIR):
    if not element in BLACKLIST:
        src_abspath = os.path.join(ROOT_DIR, element)
        dst_abspath = os.path.join(DEST_DIR, element)
        if os.path.isdir(src_abspath):
            shutil.copytree(src_abspath, dst_abspath)
        else:
            shutil.copy2(src_abspath, dst_abspath)

print("-->", DEST_DIR)
