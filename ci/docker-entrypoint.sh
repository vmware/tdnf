#!/bin/bash

set -e

test -d build && rm -rf build
mkdir -p build
cd build || exit 1

jobs=$(nproc)
cmake .. && make -j${jobs} && make python -j${jobs} && make check -j${jobs}

if ! flake8 ../pytests ; then
  echo "flake8 tests failed"
  exit 1
fi
