#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

project(tdnfmetalink VERSION 1.0.0 LANGUAGES C)

include_directories(${CMAKE_SOURCE_DIR}/include)

#make config.h with
#PACKAGE_NAME and PACKAGE_VERSION defined
configure_file(
    config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY
)

add_library(${PROJECT_NAME} SHARED
    api.c
    metalink.c
    utils.c
    list.c
)

target_link_libraries(${PROJECT_NAME}
    ${EXPAT_LIBRARIES}
    ${LIB_TDNF}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/lib)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/tdnf-plugins)
