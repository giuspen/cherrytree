#!/usr/bin/env python3

import os
import subprocess
import glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SRC_CT_DIR = os.path.join(os.path.dirname(SCRIPT_DIR), "src", "ct")
CC_FILES = glob.glob(os.path.join(SRC_CT_DIR, "*.cc"))
H_FILES = glob.glob(os.path.join(SRC_CT_DIR, "*.h"))

shell_cmd = ["xgettext",
             "--language=C++",
             "--from-code=UTF-8",
             "--keyword=_",
             "--no-location",
             "--output=" + os.path.join(SCRIPT_DIR, APP_NAME+".pot")
             ] + CC_FILES + H_FILES
subprocess.call(shell_cmd)
