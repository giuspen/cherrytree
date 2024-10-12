#!/usr/bin/env python3

import os
import shutil
import re
import subprocess
import argparse

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
DEBIAN_DIR = os.path.join(ROOT_DIR, "debian")
SCRIPTS_DIR = os.path.join(ROOT_DIR, "scripts")
ROOT_CMAKELISTS_PATH = os.path.join(ROOT_DIR, "CMakeLists.txt")
DEBIAN_CHANGELOG_PATH = os.path.join(DEBIAN_DIR, "changelog")
DEBIAN_CONTROL_PATH = os.path.join(DEBIAN_DIR, "control")
DEBIAN_RULES_PATH = os.path.join(DEBIAN_DIR, "rules")
#     package_num:  serie,      control, SHARED_FMT_SPDLOG, ninja, gtksourceview
CONTROL_DICT = {1: ("bionic",   "18.04", False,             False, 3),
                2: ("focal",    "20.04", True,              True,  4),
                3: ("jammy",    "20.04", True,              True,  4),
                4: ("noble",    "24.04", True,              True,  4),
                5: ("oracular", "24.04", True,              True,  4)}

def f_changelog_setup_for(package_num):
    changelog_lines = []
    with open(DEBIAN_CHANGELOG_PATH, "r") as fd:
        changelog_lines.extend(fd.readlines())

    changes_filename = ""
    for i in range(len(changelog_lines)):
        # cherrytree (1.0.2-2) focal; urgency=low
        match = re.search(r"cherrytree +\(([0-9]+\.[0-9]+\.[0-9]+)-[0-9]+\)", changelog_lines[i])
        if match is not None:
            changelog_lines[i] = "cherrytree ({}-{}) {}; urgency=low\n".format(match.group(1), package_num, CONTROL_DICT[package_num][0])
            changes_filename = "cherrytree_{}-{}_source.changes".format(match.group(1), package_num)
            break
    else:
        print("!! changelog version not found")
        return ""

    with open(DEBIAN_CHANGELOG_PATH, "w") as fd:
        fd.writelines(changelog_lines)
    return changes_filename

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

    for i in range(len(cmakelists_lines)):
        # pkg_check_modules(GTKSV gtksourceview-4 REQUIRED)
        if cmakelists_lines[i].find("pkg_check_modules(GTKSV gtksourceview-") >= 0:
            cmakelists_lines[i] = "pkg_check_modules(GTKSV gtksourceview-{} REQUIRED)\n".format("3.0" if 3 == CONTROL_DICT[package_num][4] else "4")
            break
    else:
        print("!! cmakelists 'pkg_check_modules(GTKSV gtksourceview-' not found")
        return False

    with open(ROOT_CMAKELISTS_PATH, "w") as fd:
        fd.writelines(cmakelists_lines)
    return True

def f_rules_setup_for(package_num):
    rules_lines = []
    with open(DEBIAN_RULES_PATH, "r") as fd:
        rules_lines.extend(fd.readlines())

    for i in range(len(rules_lines)):
        #	dh $@ --buildsystem=cmake+ninja --parallel
        if rules_lines[i].find("--buildsystem=") >= 0:
            rules_lines[i] = "\tdh $@ --buildsystem={} --parallel\n".format("cmake+ninja" if CONTROL_DICT[package_num][3] else "cmake")
            break
    else:
        print("!! rules --buildsystem= not found")
        return False

    with open(DEBIAN_RULES_PATH, "w") as fd:
        fd.writelines(rules_lines)
    return True

def f_setup_for(package_num):
    changes_filename = f_changelog_setup_for(package_num)
    if not changes_filename: return -1
    if not f_cmakelists_setup_for(package_num): return -2
    if not f_rules_setup_for(package_num): return -3
    try: shutil.copy(os.path.join(SCRIPTS_DIR, CONTROL_DICT[package_num][1], "control"), DEBIAN_CONTROL_PATH)
    except: return -4
    if 0 != subprocess.call(["debuild", "-S", "-sa", "-i", "-I", "-d"], cwd=ROOT_DIR): return -5
    if 0 != subprocess.call(["dput", "ppa:giuspen/ppa", changes_filename], cwd=os.path.dirname(ROOT_DIR)): return -6
    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate PPA package for an ubuntu serie")
    help_package_num = "Package Number { -1=all"
    for k in CONTROL_DICT.keys():
        help_package_num += " {}={}".format(k, CONTROL_DICT[k][0])
    help_package_num += " }"
    list_choices = [-1]
    list_choices.extend([k for k in CONTROL_DICT.keys()])
    parser.add_argument("package_num", type=int, choices=list_choices, help=help_package_num)
    opts = parser.parse_args()
    if opts.package_num >= 0:
        exit(f_setup_for(opts.package_num))
    else:
        for k in CONTROL_DICT.keys():
            ret_val = f_setup_for(k)
            if 0 != ret_val: exit(ret_val)
        exit(0)
