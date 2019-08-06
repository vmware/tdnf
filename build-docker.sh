#!/bin/bash

docker build -t photon/tdnf-build .
docker run --rm -it -v$(pwd):/build -w/build photon/tdnf-build

