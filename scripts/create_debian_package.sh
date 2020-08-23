#!/bin/bash
SCRIPT_FILE_PATH=$(readlink -f "$0")
SCRIPT_DIR_PATH=$(dirname "${SCRIPT_FILE_PATH}")
ROOT_DIR_PATH=$(dirname "${SCRIPT_DIR_PATH}")

cd ${ROOT_DIR_PATH}
dpkg-buildpackage -b -d -tc -j$(nproc --all)
