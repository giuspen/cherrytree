#!/usr/bin/env python3

import argparse
import os
import subprocess

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser()
parser.add_argument('language_code', help="language code (e.g. 'ru' for Russian)")
args = parser.parse_args()

shell_cmd = ("msginit",
             "--input=" + os.path.join(SCRIPT_DIR, APP_NAME+".pot"),
             "--locale=" + args.language_code + ".UTF-8")
subprocess.call(shell_cmd)
