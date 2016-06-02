/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : defines.h
 *
 * Abstract :
 *
 *            commonlib
 *
 *            common library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#pragma once

#define IsNullOrEmptyString(str) (!(str) || !(*str))

#define BAIL_ON_TDNF_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_SYSTEM_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_SYSTEM_BASE + dwError;            \
            goto error;                                            \
        }                                                          \
    } while(0)


#define TDNF_SAFE_FREE_MEMORY(pMemory) \
    do {                                                           \
        if (pMemory) {                                             \
            TDNFFreeMemory(pMemory);                               \
        }                                                          \
    } while(0)

#define TDNF_SAFE_FREE_STRINGARRAY(ppArray) \
    do {                                                           \
        if (ppArray) {                                             \
            TDNFFreeStringArray(ppArray);                          \
        }                                                          \
    } while(0)

#define TDNF_DEFAULT_MAX_STRING_LEN       16384000
