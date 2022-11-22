#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

include_directories(${CMAKE_SOURCE_DIR}/include)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config-install.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config-install.json"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/mount-small-cache.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/mount-small-cache" @ONLY
)

install(FILES config-install.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tdnf/pytests
    RENAME config.json
)

install(FILES __init__.py conftest.py
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tdnf/pytests
)

add_custom_target(check
    COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH="${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" pytest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests.."
)
