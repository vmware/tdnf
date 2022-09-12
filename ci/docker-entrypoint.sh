#!/bin/bash
tdnf install -y libmetalink-devel

rm -rf build
mkdir build && cd build
cmake .. && make && make python
make check
