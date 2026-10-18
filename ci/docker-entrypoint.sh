#!/bin/bash

set -e

build_dir="build"
HIST_DB_DIR="/usr/lib/sysimage/tdnf"

[ -d ${build_dir} ] && rm -r ${build_dir}

mkdir -p ${build_dir} ${HIST_DB_DIR}

JOBS=$(( ($(nproc)+1) / 2 ))

cmake -S . -B ${build_dir} \
  -DHISTORY_DB_DIR=${HIST_DB_DIR}

cmake --build ${build_dir} -j${JOBS}

exit_status=0

if ! make -C ${build_dir} check -j${JOBS}; then
  exit_status=1
fi

# Configure this after pytest: CMake writes test configuration into the source tree.
if ! ./ci/security-tests.sh; then
  exit_status=1
fi

if ! command -v flake8 &> /dev/null; then
  VENV_DIR="$(mktemp -d /tmp/venv.XXXXXX)"
  python3 -m venv "$VENV_DIR"
  export PATH="$VENV_DIR/bin:$PATH"
  pip install --quiet flake8
  trap 'rm -rf "$VENV_DIR"' EXIT
fi

if ! flake8 pytests; then
  echo "ERROR: flake8 tests failed" >&2
  exit_status=1
fi

exit $exit_status
