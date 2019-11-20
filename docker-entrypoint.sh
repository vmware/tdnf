#!/bin/bash

aclocal
libtoolize
automake --add-missing
autoreconf
./configure
make
make check

# pytests
make install
cd pytests && pytest --verbose
