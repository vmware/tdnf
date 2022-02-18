#!/bin/bash

rm -rf build
mkdir -p build

pushd build
cmake .. && ../scripts/build-tdnf-rpms
popd

createrepo rpms
