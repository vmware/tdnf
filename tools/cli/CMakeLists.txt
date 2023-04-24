#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

set(TDNF_BIN tdnf-bin)

add_executable(${TDNF_BIN}
    main.c
)

target_link_libraries(${TDNF_BIN}
    ${SQLITE3_LIBRARIES}
    ${LIB_TDNF_CLI}
    ${LIB_TDNF}
)

set_target_properties(${TDNF_BIN} PROPERTIES OUTPUT_NAME tdnf)

install(TARGETS ${TDNF_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binary)

add_subdirectory("lib")
