#!/bin/sh
make clean && make distclean
aclocal && libtoolize && automake --add-missing && autoreconf && ./configure --enable-python
make
