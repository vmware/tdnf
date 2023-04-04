#
# Copyright (C) 2022-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

add_library(${LIB_TDNF_LLCONF} STATIC
	entry.c
	ini.c
	lines.c
	modules.c
	nodes.c
	strutils.c
)

