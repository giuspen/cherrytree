#!/usr/bin/env python3

import os
import argparse
import glob
import subprocess
import re

DIRS_TO_EXCLUDE = ('/', '/usr/', '/usr/bin/')
LINES_TO_EXCLUDE = ['%dir "{}"'.format(dirpath) for dirpath in DIRS_TO_EXCLUDE]

def main(args):
    assert os.path.isdir(args.deb_package_dirpath)
    restore_cwd = os.getcwd()
    os.chdir(args.deb_package_dirpath)
    dep_paths = glob.glob(b'cherrytree*.deb')
    assert len(dep_paths) > 0
    #print('EXCL {}'.format(LINES_TO_EXCLUDE))
    for dep_path in dep_paths:
        console_out = subprocess.check_output(('alien', '-r', '-g', '-v', dep_path))
        re_match = re.search(b'Directory\s+(\S+)\s+prepared', console_out)
        assert re_match is not None
        exp_folder = re_match.group(1)
        exp_spec = glob.glob(os.path.join(exp_folder, b'*.spec'))[0]
        out_lines = []
        with open(exp_spec, 'r') as fd:
            for line in fd:
                line_nonl = line.replace('\n', '')
                if line_nonl not in LINES_TO_EXCLUDE:
                    out_lines.append(line)
                    #print('incl {}'.format(line_nonl))
                else:
                    print('excl {}'.format(line_nonl))
        with open(exp_spec, 'w') as fd:
            fd.writelines(out_lines)
        os.chdir(exp_folder)
        subprocess.call(('rpmbuild', '--target=noarch', '--buildroot', os.path.abspath(os.getcwd()), '-bb', os.path.basename(exp_spec)))
    os.chdir(restore_cwd)
    return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate .rpm package from existing .deb package')
    parser.add_argument('-d', '--deb_package_dirpath', default=os.path.dirname(os.getcwd()), help='Path to directory with .deb package to convert')
    import sys
    sys.exit(main(parser.parse_args()))
