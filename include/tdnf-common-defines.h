/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __INC_TDNF_COMMON_DEFINES_H__
#define __INC_TDNF_COMMON_DEFINES_H__

/* Use this to get rid of variable/parameter unused warning */
#define UNUSED(var) ((void)(var))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define IsNullOrEmptyString(str)    (!(str) || !(*str))

#define BAIL_ON_TDNF_ERROR(dwError)     \
    do {                                \
        if (dwError)                    \
        {                               \
            goto error;                 \
        }                               \
    } while(0)

#define BAIL_ON_TDNF_SYSTEM_ERROR(dwError)                  \
    do {                                                    \
        if (dwError)                                        \
        {                                                   \
            dwError = ERROR_TDNF_SYSTEM_BASE + dwError;     \
            goto error;                                     \
        }                                                   \
    } while(0)

#define TDNF_SAFE_FREE_MEMORY(pMemory)          \
    do {                                        \
        if (pMemory) {                          \
            TDNFFreeMemory(pMemory);            \
            pMemory = NULL;                     \
        }                                       \
    } while(0)

#define TDNF_SAFE_FREE_STRINGARRAY(ppArray)     \
    do {                                        \
        if (ppArray)                            \
        {                                       \
            TDNFFreeStringArray(ppArray);       \
            ppArray = NULL;                     \
        }                                       \
    } while(0)

#endif /* __INC_TDNF_COMMON_DEFINES_H__ */
