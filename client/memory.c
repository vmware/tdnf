/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : memory.c
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
#include "includes.h"

uint32_t
TDNFAllocateMemory(
    size_t size,
    void** ppMemory
    )
{
    uint32_t dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !size)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pMemory = calloc(1, size);
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

void
TDNFFreePackageInfo(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    while(pPkgInfo)
    {
        PTDNF_PKG_INFO pPkgInfoTemp = pPkgInfo;
        pPkgInfo = pPkgInfo->pNext;

        TDNFFreePackageInfoContents(pPkgInfoTemp);
        TDNFFreeMemory(pPkgInfoTemp);
    }
}

void
TDNFFreePackageInfoArray(
    PTDNF_PKG_INFO pPkgInfoArray,
    uint32_t unLength
    )
{
    uint32_t unIndex = 0;
    if(pPkgInfoArray && unLength > 0)
    {
      for(unIndex = 0; unIndex < unLength; ++unIndex)
      {
        TDNFFreePackageInfoContents(&pPkgInfoArray[unIndex]);
      }
    }
    TDNF_SAFE_FREE_MEMORY(pPkgInfoArray);
}

void
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    if(pPkgInfo)
    {
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszName);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRepoName);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszVersion);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszArch);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszSummary);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszURL);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszLicense);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszDescription);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszFormattedSize);
      TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRelease);
    }
}

void
TDNFFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    if(pSolvedPkgInfo)
    {
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsNotAvailable);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsExisting);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToInstall);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToUpgrade);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToDowngrade);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToRemove);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsUnNeeded);
       TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToReinstall);
    }
    TDNF_SAFE_FREE_MEMORY(pSolvedPkgInfo);
}
