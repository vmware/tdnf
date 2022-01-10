#!/bin/bash

rm -rf build
mkdir -p build
cd build || exit 1

cmake .. && make -j32 && make python -j32

make check -j32

if ! flake8 --ignore E501 ../pytests; then
  echo "flake8 tests failed"
  exit 1
fi
