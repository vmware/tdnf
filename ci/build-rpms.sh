#!/bin/bash
set -e

rpmdir=${1:-rpms}

rm -rf build
mkdir -p build

pushd build
cmake .. && ../scripts/build-tdnf-rpms ${rpmdir}
popd

createrepo ${rpmdir}
