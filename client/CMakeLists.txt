#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(librepo REQUIRED)

find_package(OpenSSL REQUIRED)

#make config.h with
#PACKAGE_NAME and PACKAGE_VERSION defined
configure_file(
    config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY
)

# configure pkgconfig file
configure_file(
    tdnf.pc.in
    tdnf.pc @ONLY
)

add_library(${LIB_TDNF} SHARED
    api.c
    clean.c
    client.c
    config.c
    eventdata.c
    goal.c
    gpgcheck.c
    init.c
    packageutils.c
    plugins.c
    repo.c
    repoutils.c
    remoterepo.c
    repolist.c
    resolve.c
    rpmtrans.c
    updateinfo.c
    utils.c
    validate.c
)

target_link_libraries(${LIB_TDNF}
    ${LIB_TDNF_COMMON}
    ${LIB_TDNF_SOLV}
    ${RPM_LIBRARIES}
    ${LibSolv_LIBRARIES}
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${LIBREPO_LIBRARIES}
)

set_target_properties(${LIB_TDNF} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tdnf.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS ${LIB_TDNF} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)
