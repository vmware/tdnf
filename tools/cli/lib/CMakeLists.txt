#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

# configure pkgconfig file
configure_file(
    tdnfcli.pc.in
    tdnfcli.pc @ONLY
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
    parserepolistargs.c
    parsereposyncargs.c
    parseupdateinfo.c
    updateinfocmd.c
)


set_target_properties(${LIB_TDNF_CLI} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_TDNF_CLI}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS ${LIB_TDNF_CLI} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)
