#!/usr/bin/env python2

import os, subprocess

APP_NAME = "cherrytree"
LOCALE_DIR = os.getcwd()


for dirpath, dirnames, filenames in os.walk(LOCALE_DIR):
    for filename in filenames:
        if dirpath == LOCALE_DIR:
            nation, ext = os.path.splitext(filename)
            if ext != ".po": continue
            bash_string = "msgmerge -U %s.po %s.pot" % (nation, APP_NAME)
            subprocess.call(bash_string, shell=True)
subprocess.call("rm *.po~", shell=True)
