/*
 * Copyright (C) 2017-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __SOLV_DEFINES_H__
#define __SOLV_DEFINES_H__

#define SYSTEM_REPO_NAME "@System"
#define CMDLINE_REPO_NAME "@cmdline"
#define SOLV_COOKIE_IDENT "tdnf"
#define TDNF_SOLVCACHE_DIR_NAME "solvcache"
#define SOLV_COOKIE_LEN   32

#define BAIL_ON_TDNF_LIBSOLV_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#endif /* __SOLV_DEFINES_H__ */
