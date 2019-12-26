#! /bin/bash -e

export AutogenOnlyIfNoMakefile="Y"

[[ $1 =~ ^[Rr].* ]] && (echo "*** Building Release ***" && ./autogen.sh) || (echo "*** Building Debug ***" && CXXFLAGS="-g -O0" ./autogen.sh)

# NOTE 'make check' builds *also* the unit tests
make check -j $(nproc --all)

./run_tests
