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
    "${CMAKE_CURRENT_SOURCE_DIR}/config.json" @ONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/mount-small-cache.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/mount-small-cache" @ONLY
)

add_custom_target(check
    COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH="${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" pytest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests.."
)
