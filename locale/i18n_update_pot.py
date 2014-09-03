#!/usr/bin/env python2

import os, subprocess, glob

APP_NAME = "cherrytree"

py_files = glob.glob(os.path.join(os.path.join(os.path.dirname(os.getcwd()), "modules"), "*.py"))

shell_cmd = "xgettext --language=Python --from-code=utf-8 --keyword=_ --output=%s/%s.pot %s" % (os.getcwd(), APP_NAME, " ".join(py_files))

print shell_cmd
subprocess.call(shell_cmd, shell=True)
