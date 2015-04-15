/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : strings.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#include "includes.h"

uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    )
{
    uint32_t dwError = 0;
    char* pszDst = NULL;

    if(!pszSrc || !ppszDst)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    pszDst = strdup(pszSrc);
    if(!pszDst)
    {
      dwError = ERROR_TDNF_OUT_OF_MEMORY;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszDst = pszDst;

cleanup:
    return dwError;

error:
    if(ppszDst)
    {
      *ppszDst = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszDst);
    goto cleanup;
}

uint32_t
TDNFSafeAllocateString(
    const char* pszSrc,
    char** ppszDst
    )
{
    uint32_t dwError = 0;
    char* pszDst = NULL;

    if(!ppszDst)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pszSrc)
    {
      dwError = TDNFAllocateString(pszSrc, &pszDst);
      BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszDst = pszDst;
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFAllocateStringPrintf(
    char** ppszDst,
    const char* pszFmt,
    ...
    )
{
    uint32_t dwError = 0;
    size_t nSize = 0;
    char* pszDst = NULL;
    char chDstTest = '\0';
    va_list argList;

    if(!ppszDst || !pszFmt)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Find size
    va_start(argList, pszFmt);
    nSize = vsnprintf(&chDstTest, 1, pszFmt, argList);
    va_end(argList);

    if(nSize <= 0)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    nSize = nSize + 1;
    dwError = TDNFAllocateMemory(nSize, (void**)&pszDst);
    BAIL_ON_TDNF_ERROR(dwError);

    va_start(argList, pszFmt);
    nSize = vsnprintf(pszDst, nSize, pszFmt, argList);
    va_end(argList);

    if(nSize <= 0)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    *ppszDst = pszDst;
cleanup:
    return dwError;

error:
    if(ppszDst)
    {
        *ppszDst = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszDst);
    goto cleanup;
}

void
TDNFFreeStringArray(
    char** ppszArray
    )
{
    char** ppszTemp = NULL;
    if(ppszArray)
    {
        ppszTemp = ppszArray;
        while(ppszTemp && *ppszTemp)
        {
            TDNF_SAFE_FREE_MEMORY(*ppszTemp);
            ++ppszTemp;
        }
        TDNFFreeMemory(ppszArray);
    }
}
