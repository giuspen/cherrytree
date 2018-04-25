#!/bin/bash

SCRIPT_DIR=`dirname "${0}"`
SCRIPT_DIR=`readlink -f "${SCRIPT_DIR}"`
GTKMM_DIR=`dirname "${SCRIPT_DIR}"`
SANDBOX_DIR=`dirname "${GTKMM_DIR}"`
ROOT_DIR=`dirname "${SANDBOX_DIR}"`
cp -rv ${ROOT_DIR}/future/src/7za/* src/
