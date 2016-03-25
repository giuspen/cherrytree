#!/usr/bin/env python2

import os, subprocess, glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

for po_filepath in glob.glob(os.path.join(SCRIPT_DIR, "*.po")):
    shell_cmd = ["msgmerge",
                 "-U",
                 po_filepath,
                 APP_NAME+".pot"]
    subprocess.call(shell_cmd)
shell_cmd = ["rm",
             os.path.join(SCRIPT_DIR, "*.po~")]
subprocess.call(shell_cmd)
