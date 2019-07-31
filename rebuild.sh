#!/bin/sh
make clean && make distclean
autoreconf -mif && ./configure --enable-python
make
