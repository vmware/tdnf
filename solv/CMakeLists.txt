#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(${LIB_TDNF_SOLV} STATIC
    tdnfpackage.c
    tdnfpool.c
    tdnfquery.c
    tdnfrepo.c
    simplequery.c
)
