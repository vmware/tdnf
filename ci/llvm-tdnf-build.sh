#!/bin/bash

set -e

pkgs=(llvm-devel clang-devel)
pkgs+=(which gcc cmake make)

build_dir="tdnf-llvm-build"
HIST_DB_DIR="/usr/lib/sysimage/tdnf"
JOBS=$(( ($(nproc)+1) / 2 ))

if ! rpm -q ${pkgs[@]} > /dev/null; then
  echo "Installing required build tools ..."
  if grep -qw ID=photon /etc/os-release; then
    tdnf install -y --refresh ${pkgs[@]}
  elif grep -qw ID=fedora /etc/os-release; then
    dnf install -y --refresh ${pkgs[@]}
  elif grep -qw ID=\"rocky\" /etc/os-release; then
    dnf install -y --refresh ${pkgs[@]}
  fi
fi

export CC="$(which clang)"
export CFLAGS="-Qunused-arguments -Wno-deprecated -Werror"

[ -d ${build_dir} ] && rm -r ${build_dir}

mkdir -p ${build_dir} ${HIST_DB_DIR}

cmake -S . -B ${build_dir} \
  -DHISTORY_DB_DIR=${HIST_DB_DIR}

cmake --build ${build_dir} -j${JOBS}
make -C ${build_dir} check -j${JOBS}
