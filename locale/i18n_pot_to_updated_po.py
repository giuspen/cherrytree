#!/usr/bin/env python2

import os, subprocess, glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

for po_filepath in glob.glob(os.path.join(SCRIPT_DIR, "*.po")):
    shell_cmd = ["msgmerge",
                 "-U",
                 "--backup=none",
                 po_filepath,
                 APP_NAME+".pot"]
    subprocess.call(shell_cmd)
    #subprocess.call(["zip", "-j", "-9", po_filepath+".zip", po_filepath])
