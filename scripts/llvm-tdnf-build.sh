#!/bin/bash

set -e

pkgs=(llvm-devel clang-devel)
pkgs+=(which gcc cmake make)

tdnf install -y ${pkgs[@]}

export CC="$(which clang)"
export CFLAGS="-Qunused-arguments -Wno-deprecated -Werror"

[ -d build ] && rm -rf build
mkdir -p build
cd build || exit 1

history_loc="/usr/lib/sysimage/tdnf"
p=$(nproc)

mkdir -p $history_loc
cmake -DHISTORY_DB_DIR=$history_loc .. && \
  make -j$p && \
  make check -j$p || exit 1
