#!/usr/bin/env python2

import os, subprocess

APP_NAME = "cherrytree"
LOCALE_DIR = os.getcwd()


for dirpath, dirnames, filenames in os.walk(LOCALE_DIR):
    for filename in filenames:
        if dirpath == LOCALE_DIR:
            nation, ext = os.path.splitext(filename)
            if ext != ".po": continue
            nation_dir = os.path.join(LOCALE_DIR, nation)
            if not os.path.isdir(nation_dir): os.mkdir(nation_dir)
            messages_dir = os.path.join(nation_dir, "LC_MESSAGES")
            if not os.path.isdir(messages_dir): os.mkdir(messages_dir)
            bash_string = "msgfmt --output-file=%s/LC_MESSAGES/%s.mo %s.po" % (nation, APP_NAME, nation)
            subprocess.call(bash_string, shell=True)
