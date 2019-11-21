#!/bin/bash

mkdir build && cd build
cmake ..
make
make check

