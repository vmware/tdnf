#
# Copyright (C) 2020-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

set(TDNF_CONFIG_BIN tdnf-config-bin)

add_executable(${TDNF_CONFIG_BIN}
    main.c
)

target_link_libraries(${TDNF_CONFIG_BIN}
    ${LIB_TDNF_LLCONF}
    ${LIB_TDNF_JSONDUMP}
    ${CMAKE_DL_LIBS}
)

set_target_properties(${TDNF_CONFIG_BIN} PROPERTIES OUTPUT_NAME tdnf-config)

install(TARGETS ${TDNF_CONFIG_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binary)

#add_subdirectory("lib")
