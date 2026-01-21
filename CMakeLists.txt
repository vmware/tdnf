#
# Copyright (C) 2019-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.10 FATAL_ERROR)

file(STRINGS VERSION PROJECT_VERSION)

project(tdnf VERSION ${PROJECT_VERSION} LANGUAGES C)
set(VERSION ${PROJECT_VERSION})
set(PROJECT_YEAR 2023)

# Ensure correct directory paths are used for installation
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AddCCompilerFlag)
### CFLAGS
include(CFlags)

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

if(NOT HISTORY_DB_DIR)
    set(HISTORY_DB_DIR /var/lib/tdnf)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# External dependency: rpm-devel
find_package(PkgConfig REQUIRED)
pkg_check_modules(RPM REQUIRED rpm)
find_package(OpenSSL REQUIRED)
pkg_check_modules(EXPAT REQUIRED expat)
pkg_check_modules(SQLITE3 sqlite3)
include_directories(${RPM_INCLUDE_DIRS})
include_directories(${EXPAT_INCLUDE_DIRS})

# External dependency: libsolv
find_package(LibSolv REQUIRED ext)

# External dependency: libcurl
find_package(CURL REQUIRED)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tdnf")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pytests/tests/" DESTINATION "${CMAKE_INSTALL_DATADIR}/tdnf/pytests/tests")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pytests/repo/" DESTINATION "${CMAKE_INSTALL_DATADIR}/tdnf/pytests/repo")

pkg_check_modules(RPM REQUIRED rpm)

if(RPM_VERSION VERSION_GREATER_EQUAL "6.0")
  message(STATUS "rpm ${RPM_VERSION} >= 6.0")
  message(STATUS "Enabling BUILD_WITH_RPM_6X macro")
  add_compile_definitions(BUILD_WITH_RPM_6X)
else()
  message(STATUS "rpm ${RPM_VERSION} < 6.0")
endif()

set(LIB_TDNF "tdnf")
set(LIB_TDNF_SOLV "tdnfsolv")
set(LIB_TDNF_COMMON "common")
set(LIB_TDNF_JSONDUMP "jsondump")
set(LIB_TDNF_HISTORY "history")
set(LIB_TDNF_CLI tdnfcli)
set(LIB_TDNF_LLCONF "llconf")

add_subdirectory("${PROJECT_SOURCE_DIR}/common")
add_subdirectory("${PROJECT_SOURCE_DIR}/client")
add_subdirectory("${PROJECT_SOURCE_DIR}/jsondump")
add_subdirectory("${PROJECT_SOURCE_DIR}/history")
add_subdirectory("${PROJECT_SOURCE_DIR}/llconf")
add_subdirectory("${PROJECT_SOURCE_DIR}/plugins")
add_subdirectory("${PROJECT_SOURCE_DIR}/etc")
add_subdirectory("${PROJECT_SOURCE_DIR}/bin")
add_subdirectory("${PROJECT_SOURCE_DIR}/solv")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools")
add_subdirectory("${PROJECT_SOURCE_DIR}/pytests")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/bin/tdnf-automatic.in ${PROJECT_SOURCE_DIR}/bin/tdnf-automatic @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tdnf.spec.in ${CMAKE_SOURCE_DIR}/tdnf.spec @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/scripts/build-tdnf-rpms.in ${CMAKE_SOURCE_DIR}/scripts/build-tdnf-rpms @ONLY)
