#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

set(TDNF_HISTORY_UTIL_BIN tdnf-history-util)

configure_file(
    config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

add_executable(${TDNF_HISTORY_UTIL_BIN}
    main.c
)

add_library(${LIB_TDNF_HISTORY} STATIC
    history.c
)

install(TARGETS ${TDNF_HISTORY_UTIL_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/tdnf COMPONENT binary)

target_link_libraries(${TDNF_HISTORY_UTIL_BIN}
    ${RPM_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${LIB_TDNF_HISTORY}
)

target_link_libraries(${LIB_TDNF_HISTORY}
    ${RPM_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)
