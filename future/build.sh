#!/bin/bash

[ -d build ] || mkdir build
cd build
cmake ..
make -j $(nproc --all)
./run_tests
