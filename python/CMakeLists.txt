#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

set(PYTDNF_PACKAGE_NAME "${PROJECT_NAME}")
set(PYTDNF_VERSION "${VERSION_STRING}")
set(PYTDNF_SRC_DIR "${PROJECT_SOURCE_DIR}/python")
set(PYTDNF_INC_DIR "${PROJECT_SOURCE_DIR}/include")
set(PYTDNF_LIB_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

configure_file(
    setup.py.in
    setup.py
)

add_custom_target(python
    COMMAND cp -r "${CMAKE_CURRENT_SOURCE_DIR}/tdnf" .
    COMMAND ${PYTHON_EXECUTABLE} setup.py build
)

add_custom_target(package_pytdnf
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist --formats=rpm
)
