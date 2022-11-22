#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

set(TDNF_JSON_BIN jsondumptest)

add_executable(${TDNF_JSON_BIN}
    test.c
)

install(TARGETS ${TDNF_JSON_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binary)

add_library(${LIB_TDNF_JSONDUMP} STATIC
    jsondump.c
)

target_link_libraries(${TDNF_JSON_BIN}
    ${LIB_TDNF_JSONDUMP}
)
