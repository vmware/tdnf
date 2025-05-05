/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFAllocateMemory(
    size_t nNumElements,
    size_t nSize,
    void** ppMemory
    )
{
    uint32_t dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !nSize || !nNumElements)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nNumElements > SIZE_MAX/nSize)
    {
        dwError = ERROR_TDNF_INVALID_ALLOCSIZE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pMemory = calloc(nNumElements, nSize);
    if (!pMemory)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppMemory = pMemory;

cleanup:
    return dwError;

error:
    if (ppMemory)
    {
        *ppMemory = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pMemory);
    goto cleanup;
}

uint32_t
TDNFReAllocateMemory(
    size_t nSize,
    void** ppMemory
    )
{
    uint32_t dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !nSize)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pMemory = realloc(*ppMemory, nSize);
    if (!pMemory)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppMemory = pMemory;

cleanup:
    return dwError;

error:
    if (ppMemory)
    {
        free(*ppMemory);
        *ppMemory = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pMemory);
    goto cleanup;
}

void
TDNFFreeMemory(
    void* pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }
}
