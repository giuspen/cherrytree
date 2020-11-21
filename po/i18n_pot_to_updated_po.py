#!/usr/bin/env python3

import argparse
import os
import subprocess
import glob

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser()
parser.add_argument('-z', '--zip', action='store_true', help='whether to zip the .po files after generating')
args = parser.parse_args()

for po_filepath in glob.glob(os.path.join(SCRIPT_DIR, "*.po")):
    shell_cmd = ("msgmerge",
                 "-U",
                 "--backup=none",
                 po_filepath,
                 os.path.join(os.path.dirname(po_filepath), APP_NAME+".pot"))
    subprocess.call(shell_cmd)
    if args.zip:
        subprocess.call(["zip", "-j", "-9", po_filepath+".zip", po_filepath])
