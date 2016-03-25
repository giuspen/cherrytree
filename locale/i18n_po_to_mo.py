#!/usr/bin/env python2

import os, subprocess, glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

for po_filepath in glob.glob(os.path.join(SCRIPT_DIR, "*.po")):
    po_filename = os.path.basename(po_filepath)
    nation = po_filename[:-3]
    nation_dir = os.path.join(SCRIPT_DIR, nation)
    if not os.path.isdir(nation_dir): os.mkdir(nation_dir)
    messages_dir = os.path.join(nation_dir, "LC_MESSAGES")
    if not os.path.isdir(messages_dir): os.mkdir(messages_dir)
    shell_cmd = ["msgfmt",
                 "--output-file=%s" % os.path.join(SCRIPT_DIR, nation, "LC_MESSAGES", APP_NAME+".mo"),
                 nation+".po"]
    subprocess.call(shell_cmd)
