#!/usr/bin/env python3

import os
import shutil
import re
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEBIAN_DIR = os.path.join(ROOT_DIR, "debian")
SCRIPTS_DIR = os.path.join(ROOT_DIR, "scripts")
ROOT_CMAKELISTS_PATH = os.path.join(ROOT_DIR, "CMakeLists.txt")
DEBIAN_CHANGELOG_PATH = os.path.join(DEBIAN_DIR, "changelog")
DEBIAN_CONTROL_PATH = os.path.join(DEBIAN_DIR, "control")
#     package_num:  serie,    control, SHARED_FMT_SPDLOG
CONTROL_DICT = {1: ("bionic", "18.04", False),
                2: ("focal",  "20.04", True),
                3: ("jammy",  "20.04", True),
                4: ("lunar",  "22.10", True),
                5: ("mantic", "22.10", True)}

def f_changelog_setup_for(package_num):
    changelog_lines = []
    with open(DEBIAN_CHANGELOG_PATH, "r") as fd:
        changelog_lines.extend(fd.readlines())

    for i in range(len(changelog_lines)):
        # cherrytree (1.0.2-2) focal; urgency=low
        match = re.search(r"cherrytree +\(([0-9]+\.[0-9]+\.[0-9]+)-[0-9]+\)", changelog_lines[i])
        if match is not None:
            changelog_lines[i] = "cherrytree ({}-{}) {}; urgency=low\n".format(match.group(1), package_num, CONTROL_DICT[package_num][0])
            break
    else:
        print("!! changelog version not found")
        return False

    with open(DEBIAN_CHANGELOG_PATH, "w") as fd:
        fd.writelines(changelog_lines)
    return True

def f_cmakelists_setup_for(package_num):
    cmakelists_lines = []
    with open(ROOT_CMAKELISTS_PATH, "r") as fd:
        cmakelists_lines.extend(fd.readlines())

    for i in range(len(cmakelists_lines)):
        # option(USE_SHARED_FMT_SPDLOG "Use shared fmt and spdlog (not those bundled)" ON)
        if cmakelists_lines[i].find("option(USE_SHARED_FMT_SPDLOG ") >= 0:
            cmakelists_lines[i] = "option(USE_SHARED_FMT_SPDLOG \"Use shared fmt and spdlog (not those bundled)\" {})\n".format("ON" if CONTROL_DICT[package_num][2] else "OFF")
            break
    else:
        print("!! cmakelists option USE_SHARED_FMT_SPDLOG not found")
        return False

    with open(ROOT_CMAKELISTS_PATH, "w") as fd:
        fd.writelines(cmakelists_lines)
    return True

def f_setup_for(package_num):
    f_changelog_setup_for(package_num)
    f_cmakelists_setup_for(package_num)
    shutil.copy(os.path.join(SCRIPTS_DIR, CONTROL_DICT[package_num][1], "control"), DEBIAN_CONTROL_PATH)

def main(args):
    f_setup_for(2)

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
