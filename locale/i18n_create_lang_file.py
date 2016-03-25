#!/usr/bin/env python2

import argparse, os, subprocess

APP_NAME = "cherrytree"
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

parser = argparse.ArgumentParser()
parser.add_argument('language_code', help="language code (e.g. 'es' for Spanish)")
args = parser.parse_args()

shell_cmd = ["msginit",
             "--input=%s" % os.path.join(SCRIPT_DIR, APP_NAME+".pot"),
             "--locale="+args.language_code]
subprocess.call(shell_cmd)
