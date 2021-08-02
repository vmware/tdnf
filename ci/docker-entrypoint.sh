#!/bin/bash

rm -rf build
mkdir -p build
cd build || exit 1
cmake .. && make -j32 && make python -j32
make check -j32
