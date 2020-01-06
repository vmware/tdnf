#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

project(tdnf VERSION 2.0.0 LANGUAGES C)
set(PROJECT_YEAR 2019)

# Ensure correct directory paths are used for installation
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AddCCompilerFlag)
include(BuildMetaData)
include(CreatePackages)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Ensure that we don't accidentally install into /usr/etc
set(CMAKE_INSTALL_FULL_SYSCONDIR "/etc")

##
## C Flags
##

add_c_compiler_flag(-Wall)
add_c_compiler_flag(-Wextra)
add_c_compiler_flag(-std=gnu99)

add_c_compiler_flag(-O3 RELEASE)
add_c_compiler_flag(-s RELEASE)
add_c_compiler_flag(-O0 DEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Custom command produces build_meta_data.c, and a dummy output that's
# not actually produced, in order to force Version.cmake to always be
# re-run before the build
ADD_CUSTOM_COMMAND(
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/build_meta_data.c
            ${CMAKE_CURRENT_BINARY_DIR}/_build_meta_data.c
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildMetaData.cmake
    COMMENT "Generating build meta data.."
)

##
## build libtdnfclient.so
##

### External dependency: rpm-devel
find_package(PkgConfig REQUIRED)
pkg_check_modules(RPM REQUIRED rpm)
include_directories(${RPM_INCLUDE_DIRS})

### External dependency: libsolv
find_package(LibSolv REQUIRED ext)

### External dependency: libcurl
find_package(CURL REQUIRED)

set(LIB_TDNF_CLIENT tdnfclient)

list(APPEND LIB_TDNF_COMMMON_SRC
    "${CMAKE_BINARY_DIR}/build_meta_data.c"
    "common/memory.c"
    "common/strings.c"
    "common/configreader.c"
    "common/setopt.c"
    "common/utils.c"
)

list(APPEND LIB_TDNF_SOLV_SRC
    "solv/tdnfpool.c"
    "solv/tdnfrepo.c"
    "solv/tdnfquery.c"
    "solv/tdnfpackage.c"
)

list(APPEND LIB_TDNF_CLIENT_SRC
    "client/api.c"
    "client/clean.c"
    "client/client.c"
    "client/config.c"
    "client/goal.c"
    "client/gpgcheck.c"
    "client/init.c"
    "client/packageutils.c"
    "client/repo.c"
    "client/remoterepo.c"
    "client/repolist.c"
    "client/repoutils.c"
    "client/resolve.c"
    "client/rpmtrans.c"
    "client/updateinfo.c"
    "client/utils.c"
    "client/validate.c"
)

add_library(${LIB_TDNF_CLIENT} SHARED
    ${LIB_TDNF_CLIENT_SRC}
    ${LIB_TDNF_SOLV_SRC}
    ${LIB_TDNF_COMMMON_SRC}
)

set_target_properties(${LIB_TDNF_CLIENT} PROPERTIES
    OUTPUT_NAME "tdnf"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS ${LIB_TDNF_CLIENT} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)

# configure and install pkgconfig file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/client/tdnf.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/client/tdnf.pc" @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/client/tdnf.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

##
## build libtdnfcli.so
##

set(LIB_TDNF_CLI tdnfcli)

list(APPEND LIB_TDNF_CLI_SRC
    "tools/cli/lib/api.c"
    "tools/cli/lib/help.c"
    "tools/cli/lib/installcmd.c"
    "tools/cli/lib/options.c"
    "tools/cli/lib/output.c"
    "tools/cli/lib/parseargs.c"
    "tools/cli/lib/parsecleanargs.c"
    "tools/cli/lib/parselistargs.c"
    "tools/cli/lib/parserepolistargs.c"
    "tools/cli/lib/parseupdateinfo.c"
    "tools/cli/lib/updateinfocmd.c"
)

add_library(${LIB_TDNF_CLI} SHARED ${LIB_TDNF_CLI_SRC})

set_target_properties(${LIB_TDNF_CLI} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS ${LIB_TDNF_CLI} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)

# configure and install pkgconfig file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/cli/lib/tdnfcli.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tools/cli/lib/tdnfcli.pc" @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tools/cli/lib/tdnfcli.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

##
## libtdnf + libtdnfcli headers
##

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tdnf")

##
## tdnf binary
##

set(TDNF_BIN tdnf)

list(APPEND TDNF_BIN_SRC
    "tools/cli/main.c"
)

add_executable(${TDNF_BIN} ${TDNF_BIN_SRC})

target_link_libraries(${TDNF_BIN} PUBLIC
    ${LIB_TDNF_CLIENT}
    ${LIB_TDNF_CLI}
    ${RPM_LIBRARIES}
    ${LibSolv_LIBRARIES}
    ${CURL_LIBRARIES}
)

install(TARGETS ${TDNF_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binary)

##
## Other static resources
##

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/tdnf.conf"
    DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/tdnf"
    COMPONENT resources
)

install(DIRECTORY DESTINATION "share/bash-completion/completions")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/tdnf-completion.bash"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/bash-completion/completions"
    RENAME "tdnf"
    COMPONENT resources
)

##
## check target
##
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/pytests/config.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/pytests/config.json" @ONLY
)

add_custom_target(check
    COMMAND pytest --build-dir=${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pytests
    COMMENT "Running tests.."
)

##
## python modules
##

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

set(PYTDNF_PACKAGE_NAME "${PROJECT_NAME}")
set(PYTDNF_VERSION "${VERSION_STRING}")
set(PYTDNF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python")
set(PYTDNF_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PYTDNF_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/python/setup.py.in"
    "${CMAKE_CURRENT_BINARY_DIR}/python/setup.py"
)

add_custom_target(python
    COMMAND cp -r "${CMAKE_CURRENT_SOURCE_DIR}/python/tdnf" .
    COMMAND ${PYTHON_EXECUTABLE} setup.py build
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/python"
)

add_custom_target(package_pytdnf
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist --formats=rpm
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/python"
)

