#!/usr/bin/env python2

import os, subprocess, glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
MODULES_DIR = os.path.join(os.path.dirname(SCRIPT_DIR), "modules")
PY_FILES = glob.glob(os.path.join(MODULES_DIR, "*.py"))

shell_cmd = ["xgettext",
             "--language=Python",
             "--from-code=utf-8",
             "--keyword=_",
             "--output=%s" % os.path.join(SCRIPT_DIR, APP_NAME+".pot")
             ] + PY_FILES
subprocess.call(shell_cmd)
