#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

# configure pkgconfig file
configure_file(
    tdnf-cli-libs.pc.in
    tdnf-cli-libs.pc @ONLY
)

add_library(${LIB_TDNF_CLI} SHARED
    api.c
    help.c
    installcmd.c
    options.c
    output.c
    parseargs.c
    parsecleanargs.c
    parselistargs.c
    parsehistoryargs.c
    parserepolistargs.c
    parserepoqueryargs.c
    parsereposyncargs.c
    parseupdateinfo.c
    updateinfocmd.c
)

target_link_libraries(${LIB_TDNF_CLI}
    ${LIB_TDNF_JSONDUMP}
)

set_target_properties(${LIB_TDNF_CLI} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tdnf-cli-libs.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS ${LIB_TDNF_CLI} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)
