/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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
TDNFStringSepCount(
    char *pszBuf,
    char *pszSep,
    size_t *nSepCount
    )
{
    size_t nCount = 0;
    uint32_t dwError = 0;
    const char *pszTemp = NULL;

    if (!pszBuf || IsNullOrEmptyString(pszSep))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTemp = pszBuf;
    while (*pszTemp)
    {
        while (*pszTemp && strchr(pszSep, *pszTemp))
        {
            pszTemp++;
        }
        if (*pszTemp == '\0')
        {
            break;
        }
        nCount++;
        while(*pszTemp && !strchr(pszSep, *pszTemp))
        {
            pszTemp++;
        }
    }

    *nSepCount = nCount;

error:
    return dwError;
}

uint32_t
TDNFSplitStringToArray(
    char *pszBuf,
    char *pszSep,
    char ***pppszTokens
    )
{
    uint32_t dwError = 0;
    char *pszTok = NULL;
    char *pszState = NULL;
    char **ppszToks = NULL;
    size_t nCount = 0;
    size_t nIndex = 0;

    if (!pszBuf || IsNullOrEmptyString(pszSep))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFStringSepCount(pszBuf, pszSep, &nCount);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(nCount + 1, sizeof(char *), (void**)&ppszToks);
    BAIL_ON_TDNF_ERROR(dwError);

    pszTok = strtok_r(pszBuf, pszSep, &pszState);
    while (nIndex < nCount && pszTok != NULL)
    {
        dwError = TDNFAllocateString(pszTok, &ppszToks[nIndex]);
        BAIL_ON_TDNF_ERROR(dwError);
        nIndex += 1;
        pszTok = strtok_r(NULL, pszSep, &pszState);
    }
    ppszToks[nIndex] = NULL;
    *pppszTokens = ppszToks;

cleanup:
    return dwError;

error:
    TDNFFreeStringArrayWithCount(ppszToks, nCount);
    *pppszTokens = NULL;
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
    int32_t nSize = 0;
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
    nSize++;

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

uint32_t
TDNFAllocateStringArray(
    char** ppszSrc,
    char*** pppszDst
    )
{
    uint32_t dwError = 0;
    char** ppszDst = NULL;
    int i, n = 0;

    if(!ppszSrc || !pppszDst)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (i = 0; ppszSrc[i]; i++) {
        n++;
    }
    dwError = TDNFAllocateMemory(n + 1, sizeof(char *), (void **)&ppszDst);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 0; i < n; i++) {
        dwError = TDNFSafeAllocateString(
                      ppszSrc[i],
                      &ppszDst[i]);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *pppszDst = ppszDst;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_STRINGARRAY(ppszDst);
    if(pppszDst) {
        *pppszDst = NULL;
    }
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

uint32_t
TDNFTrimSuffix(
    char* pszSource,
    const char* pszSuffix
    )
{
    uint32_t dwError = 0;
    int nSourceStrLen = 0, nSuffixStrLen = 0;

    if (IsNullOrEmptyString(pszSource) || IsNullOrEmptyString(pszSuffix))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSourceStrLen = strlen(pszSource);
    nSuffixStrLen = strlen(pszSuffix);

    if (nSuffixStrLen > nSourceStrLen)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while (nSuffixStrLen > 0 &&
          (pszSource[nSourceStrLen - 1] == pszSuffix[nSuffixStrLen - 1]))
    {
        nSourceStrLen--;
        nSuffixStrLen--;
    }

    pszSource[nSourceStrLen] = '\0';
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFStringEndsWith(
    char* pszSource,
    const char* pszSuffix
    )
{
    int nSourceStrLen = 0, nSuffixStrLen = 0;
    uint32_t dwError = 0;
    int ret = 0;

    if (IsNullOrEmptyString(pszSource) || IsNullOrEmptyString(pszSuffix))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSourceStrLen = strlen(pszSource);
    nSuffixStrLen = strlen(pszSuffix);

    if (nSuffixStrLen > nSourceStrLen)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ret = strncmp(pszSource + nSourceStrLen - nSuffixStrLen, pszSuffix, nSuffixStrLen);

    if (ret)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFStringArrayCount(
    char **ppszStringArray,
    int *pnCount
)
{
    uint32_t dwError = 0;
    int nCount;

    if (!ppszStringArray || !pnCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(nCount = 0; ppszStringArray[nCount]; nCount++);

    *pnCount = nCount;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static int
_cmpstringp(const void *p1, const void *p2)
{
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

uint32_t
TDNFStringArraySort(
    char **ppszArray
)
{
    uint32_t dwError = 0;
    int nCount;

    if (!ppszArray)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(nCount = 0; ppszArray[nCount]; nCount++);

    qsort(ppszArray, nCount, sizeof(char *), _cmpstringp);

cleanup:
    return dwError;
error:
    goto cleanup;
}

