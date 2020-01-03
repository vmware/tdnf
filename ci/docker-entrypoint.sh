#!/bin/bash

rm -rf build
mkdir build && cd build
cmake .. && make && make python

if [ "${DIST}" = "photon" ]; then
# the tests only work on Photon OS right now...
make check
fi
