#!/bin/bash
set -e

rm -rf build
mkdir -p build
cd build || exit 1

mkdir -p /usr/lib/sysimage/tdnf
cmake -DHISTORY_DB_DIR=/usr/lib/sysimage/tdnf .. && make -j32 && make check -j32 || exit

if ! flake8 ../pytests ; then
  echo "flake8 tests failed"
  exit 1
fi
