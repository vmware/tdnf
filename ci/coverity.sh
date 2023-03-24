#!/bin/sh

COVERITY_BIN=/coverity/bin/
export PATH=${COVERITY_BIN}:${PATH}

COVERITY_DIR=build-coverity
rm -rf ${COVERITY_DIR}
mkdir -p ${COVERITY_DIR}
cd ${COVERITY_DIR}

cmake ..

COV_CONFIG=coverity-config.xml
COV_DIR=coverity-intermediate
CC=cc

cov-configure --config ${COV_CONFIG} --compiler ${CC} --comptype gcc --template
cov-build --dir ${COV_DIR} --config ${COV_CONFIG} make

# gcc >= 10.0 workaround, see https://www.mail-archive.com/gcc@gcc.gnu.org/msg93814.html
#if grep -qr 'version>0' . --include=*.xml ; then
#    for f in $(grep -lr 'version>0' . --include=*.xml) ; do sed -i 's/version>0/version>10/' $f; done
#    make clean
#    cov-build --dir ${COV_DIR} --config ${COV_CONFIG} make
#fi

cov-analyze --dir ${COV_DIR} --config ${COV_CONFIG} --all

mkdir html
cov-format-errors --dir ${COV_DIR} --html-output html

