/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

#define STRINGIFYX(N) STRINGIFY(N)
#define STRINGIFY(N) #N

#define MAX_CONFIG_LINE_LENGTH      1024

#define TDNF_SAFE_FREE_PKGINFO(pPkgInfo) \
    do {                                                           \
        if (pPkgInfo) {                                            \
            TDNFFreePackageInfo(pPkgInfo);                         \
        }                                                          \
    } while(0)

#define TDNF_DEFAULT_MAX_STRING_LEN       16384000
