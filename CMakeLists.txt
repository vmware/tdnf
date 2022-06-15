#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

project(tdnf VERSION 3.1.9 LANGUAGES C)
set(VERSION ${PROJECT_VERSION})
set(PROJECT_YEAR 2022)

# Ensure correct directory paths are used for installation
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AddCCompilerFlag)
include(CreatePackages)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Ensure that we don't accidentally install into /usr/etc
set(CMAKE_INSTALL_FULL_SYSCONDIR "/etc")
set(SYSCONFDIR /etc)

set(MOTGEN_DIR /etc/motdgen.d)

if(NOT SYSTEMD_DIR)
    set(SYSTEMD_DIR /lib/systemd/system)
endif()


##
## C Flags
##

add_c_compiler_flag(-Wall)
add_c_compiler_flag(-Wextra)
add_c_compiler_flag(-std=gnu99)
add_c_compiler_flag(-fPIC)

add_c_compiler_flag(-O3 RELEASE)
add_c_compiler_flag(-s RELEASE)
add_c_compiler_flag(-g DEBUG)

add_c_compiler_flag(-D_XOPEN_SOURCE=500)
add_c_compiler_flag(-D_DEFAULT_SOURCE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

### External dependency: rpm-devel
find_package(PkgConfig REQUIRED)
pkg_check_modules(RPM REQUIRED rpm)
include_directories(${RPM_INCLUDE_DIRS})

### External dependency: libsolv
find_package(LibSolv REQUIRED ext)

### External dependency: libcurl
find_package(CURL REQUIRED)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tdnf")

set(LIB_TDNF "tdnf")
set(LIB_TDNF_SOLV "tdnfsolv")
set(LIB_TDNF_COMMON "common")
set(LIB_TDNF_CLI tdnfcli)

add_subdirectory("${PROJECT_SOURCE_DIR}/common")
add_subdirectory("${PROJECT_SOURCE_DIR}/client")
add_subdirectory("${PROJECT_SOURCE_DIR}/plugins")
add_subdirectory("${PROJECT_SOURCE_DIR}/python")
add_subdirectory("${PROJECT_SOURCE_DIR}/etc")
add_subdirectory("${PROJECT_SOURCE_DIR}/bin")
add_subdirectory("${PROJECT_SOURCE_DIR}/solv")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools")
add_subdirectory("${PROJECT_SOURCE_DIR}/pytests")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/bin/tdnf-automatic.in ${PROJECT_SOURCE_DIR}/bin/tdnf-automatic @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/bin/tdnf-cache-updateinfo.in ${PROJECT_SOURCE_DIR}/bin/tdnf-cache-updateinfo @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tdnf.spec.in ${CMAKE_SOURCE_DIR}/tdnf.spec @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/scripts/build-tdnf-rpms.in ${CMAKE_SOURCE_DIR}/scripts/build-tdnf-rpms @ONLY)
