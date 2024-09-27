/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
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
    const char *pszBuf,
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
    const char *pszBuf,
    char *pszSep,
    char ***pppszTokens
    )
{
    uint32_t dwError = 0;
    char **ppszToks = NULL;
    const char *p;
    size_t i, n;

    if (!pszBuf || IsNullOrEmptyString(pszSep) || !pppszTokens)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFStringSepCount(pszBuf, pszSep, &n);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(n + 1, sizeof(char *), (void**)&ppszToks);
    BAIL_ON_TDNF_ERROR(dwError);

    i = 0;
    p = pszBuf;
    while(*p) {
        const char *p0;
        while (*p && strchr(pszSep, *p))
            p++;
        if (!*p)
            break;
        p0 = p;
        while (*p && !strchr(pszSep, *p))
            p++;
        if((ppszToks[i++] = strndup(p0, p - p0)) == NULL) {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    ppszToks[i] = NULL;
    *pppszTokens = ppszToks;

cleanup:
    return dwError;

error:
    TDNFFreeStringArrayWithCount(ppszToks, n);
    *pppszTokens = NULL;
    goto cleanup;
}

uint32_t
TDNFMergeStringArrays(
    char ***pppszArray0,
    char **ppszArray1
)
{
    uint32_t dwError = 0;
    int i, n, n0, n1;

    if (!pppszArray0 || !ppszArray1) {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFStringArrayCount(*pppszArray0, &n0);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFStringArrayCount(ppszArray1, &n1);
    BAIL_ON_TDNF_ERROR(dwError);

    n = n0 + n1;

    dwError = TDNFReAllocateMemory((n + 1) * sizeof(char *), (void **)pppszArray0);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 0; i < n1; i++) {
        (*pppszArray0)[n0 + i] = ppszArray1[i];
    }
    (*pppszArray0)[n] = NULL;

    TDNF_SAFE_FREE_MEMORY(ppszArray1);

cleanup:
    return dwError;
error:
    goto cleanup;
}

/* adds a space separated list of strings to an existying string array */
uint32_t
TDNFAddStringArray(
    char ***pppszArray,
    char *pszValue)
{
    uint32_t dwError = 0;
    char **ppszArrayToAdd = NULL;

    if(!pppszArray) {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszValue)) {
         /* setting to an empty value resets it (eg. "setopt=foo=") */
        TDNF_SAFE_FREE_STRINGARRAY(*pppszArray);
    } else {
        dwError = TDNFSplitStringToArray(pszValue,
                                         " ", &ppszArrayToAdd);
        BAIL_ON_TDNF_ERROR(dwError);

        if (*pppszArray != NULL) {
            dwError = TDNFMergeStringArrays(pppszArray, ppszArrayToAdd);
            BAIL_ON_TDNF_ERROR(dwError);
        } else {
            /* if first list is empty, just set to what we add */
            *pppszArray = ppszArrayToAdd;
        }
    }

cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_STRINGARRAY(ppszArrayToAdd);
    goto cleanup;
}

uint32_t
TDNFJoinArrayToString(
    char **ppszArray,
    const char *pszSep,
    int count,
    char **ppszResult
)
{
    uint32_t dwError = 0;
    int i, p = 0;
    int nSize = 0, nSepSize = 0;
    char *pszResult = NULL;

    if(!ppszArray || !pszSep || !ppszResult)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSepSize = strlen(pszSep);
    nSize = nSepSize * count;
    for(i = 0; i < count && ppszArray[i]; i++)
    {
        nSize += strlen(ppszArray[i]);
    }
    nSize++;

    dwError = TDNFAllocateMemory(nSize, sizeof(char), (void**)&pszResult);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i < count && ppszArray[i]; i++)
    {
        strcpy(&pszResult[p], ppszArray[i]);
        p += strlen(ppszArray[i]);
        if (i < count-1 && ppszArray[i+1])
        {
            strcpy(&pszResult[p], pszSep);
            p += nSepSize;
        }
    }

    *ppszResult = pszResult;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszResult);
    goto cleanup;
}

uint32_t
TDNFJoinArrayToStringSorted(
    char **ppszDependencies,
    const char *pszSep,
    char **ppszResult
)
{
    int nCount = 0, p = 0, k, i;
    char **ppszLines = NULL;
    uint32_t dwError = 0;
    int nSepSize = 0, nSize = 0;
    char *pszResult = NULL;

    if (ppszDependencies)
    {
        for (i = 0; ppszDependencies[i]; i++);
        nCount += i;
    }

    if (nCount > 0)
    {
        dwError = TDNFAllocateMemory(nCount + 1, sizeof(char *), (void**)&ppszLines);
        BAIL_ON_TDNF_ERROR(dwError);

        if (ppszDependencies)
        {
            k = 0;
            for (i = 0; ppszDependencies[i]; i++)
            {
                ppszLines[k++] = ppszDependencies[i];
            }
        }
        dwError = TDNFStringArraySort(ppszLines);
        BAIL_ON_TDNF_ERROR(dwError);

        nSepSize = strlen(pszSep);
        nSize = nSepSize * (nCount + 1);

        for(i = 0; ppszLines[i]; i++)
        {
            if(i == 0 || strcmp(ppszLines[i], ppszLines[i-1]))
              nSize += strlen(ppszLines[i]);
        }
        nSize++;

        dwError = TDNFAllocateMemory(nSize, sizeof(char), (void**)&pszResult);
        BAIL_ON_TDNF_ERROR(dwError);

        for (i = 0; ppszLines[i]; i++)
        {
            if (i == 0)
            {
                 strcpy(&pszResult[p], ppszLines[i]);
                 p += strlen(ppszLines[i]);
            } else if (strcmp(ppszLines[i], ppszLines[i-1]))
            {
                 strcpy(&pszResult[p], pszSep);
                 p += nSepSize;
                 strcpy(&pszResult[p], ppszLines[i]);
                 p += strlen(ppszLines[i]);
            }
        }
    }

    if (pszResult) {
        *ppszResult = strdup(pszResult);
    } else {
        *ppszResult = strdup("");
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(ppszLines);
    TDNF_SAFE_FREE_MEMORY(pszResult);
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

uint32_t TDNFTrimSuffix(char *pszSource, const char *pszSuffix)
{
    char *ptr;

    if (IsNullOrEmptyString(pszSource) || IsNullOrEmptyString(pszSuffix)) {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    ptr = strstr(pszSource, pszSuffix);
    if (!ptr || strcmp(ptr, pszSuffix)) {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    *ptr = '\0';
    return 0;
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