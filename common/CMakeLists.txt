#
# Copyright (C) 2020-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(${LIB_TDNF_COMMON} STATIC
    memory.c
    strings.c
    utils.c
    log.c
    lock.c
)

# Apply appropriate flags for static library
apply_tdnf_flags(${LIB_TDNF_COMMON} STATIC)

set_target_properties(${LIB_TDNF_COMMON} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    INTERFACE_POSITION_INDEPENDENT_CODE ON
)
