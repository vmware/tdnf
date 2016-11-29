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

    if(strlen(pszSrc) > TDNF_DEFAULT_MAX_STRING_LEN)
    {
        dwError = ERROR_TDNF_STRING_TOO_LONG;
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

    if(nSize > TDNF_DEFAULT_MAX_STRING_LEN)
    {
        dwError = ERROR_TDNF_STRING_TOO_LONG;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, nSize, (void**)&pszDst);
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

void
TDNFFreeStringArrayWithCount(
    char **ppszArray,
    int nCount
    )
{
    if(ppszArray)
    {
        while(nCount)
        {
            TDNFFreeMemory(ppszArray[--nCount]);
        }
        TDNFFreeMemory(ppszArray);
    }
}

uint32_t
TDNFAllocateStringN(
    const char* pszSrc,
    uint32_t dwNumElements,
    char** ppszDst
    )
{
    uint32_t dwError = 0;
    char* pszDst = NULL;
    uint32_t dwSrcLength = 0;

    if(!pszSrc || !ppszDst)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    dwSrcLength = strlen(pszSrc);
    if(dwNumElements > dwSrcLength)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(dwNumElements + 1, 1, (void**)&pszDst);
    BAIL_ON_TDNF_ERROR(dwError);

    strncpy(pszDst, pszSrc, dwNumElements);

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
TDNFReplaceString(
    const char* pszSource,
    const char* pszSearch,
    const char* pszReplace,
    char** ppszDst
    )
{
    uint32_t dwError = 0;
    char* pszDst = NULL;
    char* pszTemp = NULL;
    char* pszTempReplace = NULL;
    char* pszIndex = NULL;
    int nSearchLen = 0;

    if(IsNullOrEmptyString(pszSource) ||
       IsNullOrEmptyString(pszSearch) ||
       !pszReplace ||
       !ppszDst)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSearchLen = strlen(pszSearch);

    dwError = TDNFAllocateString(pszSource, &pszDst);
    BAIL_ON_TDNF_ERROR(dwError);

    while((pszIndex = strstr(pszDst, pszSearch)) != NULL)
    {
        dwError = TDNFAllocateStringN(
                      pszDst,
                      pszIndex - pszDst,
                      &pszTemp);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateStringPrintf(
                      &pszTempReplace,
                      "%s%s%s",
                      pszTemp,
                      pszReplace,
                      pszIndex + nSearchLen);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszTemp);
        TDNF_SAFE_FREE_MEMORY(pszDst);
        pszDst = pszTempReplace;
        pszTemp = NULL;
        pszTempReplace = NULL;
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
    TDNF_SAFE_FREE_MEMORY(pszTemp);
    TDNF_SAFE_FREE_MEMORY(pszTempReplace);
    goto cleanup;
}
