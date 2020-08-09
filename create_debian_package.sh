#!/bin/sh

dpkg-buildpackage -b -d -tc -j$(nproc --all)
