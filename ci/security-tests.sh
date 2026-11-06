#!/bin/bash

set -eu

build_dir="${1:-build-security}"
jobs=$(( ($(nproc)+1) / 2 ))

# Instrument the libraries as well as the tests, with PIC for static libraries
# that are linked into libtdnf. Build only the focused regression targets.
cmake -S . -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build "$build_dir" -j"$jobs" \
  --target security-client security-metalink security-solver

ASAN_OPTIONS=detect_leaks=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --test-dir "$build_dir" --output-on-failure
