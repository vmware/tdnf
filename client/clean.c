/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : clean.c
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

static
uint32_t
TDNFCopyEnabledRepos(
    PTDNF_REPO_DATA pRepoData,
    char*** pppszReposUsed
    );

uint32_t
TDNFClean(
    PTDNF pTdnf,
    TDNF_CLEANTYPE nCleanType,
    PTDNF_CLEAN_INFO* ppCleanInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_CLEAN_INFO pCleanInfo = NULL;
    char** ppszReposUsed = NULL;

    if(!pTdnf || !ppCleanInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                sizeof(TDNF_CLEAN_INFO),
                (void**)&pCleanInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCopyEnabledRepos(pTdnf->pRepos, &pCleanInfo->ppszReposUsed);
    BAIL_ON_TDNF_ERROR(dwError);

    ppszReposUsed = pCleanInfo->ppszReposUsed;
    if(nCleanType == CLEANTYPE_ALL)
    {
        while(*ppszReposUsed)
        {
            dwError = TDNFRepoRemoveCache(pTdnf, *ppszReposUsed);
            BAIL_ON_TDNF_ERROR(dwError);

            ++ppszReposUsed;
        }
    }

    pCleanInfo->nCleanAll = (nCleanType == CLEANTYPE_ALL);

    *ppCleanInfo = pCleanInfo;
cleanup:
    return dwError;

error:
    if(ppCleanInfo)
    {
        *ppCleanInfo = NULL;
    }
    if(pCleanInfo)
    {
        TDNFFreeCleanInfo(pCleanInfo);
    }
    goto cleanup;
}

uint32_t
TDNFCopyEnabledRepos(
    PTDNF_REPO_DATA pRepoData,
    char*** pppszReposUsed
    )
{
    uint32_t dwError = 0;
    char** ppszReposUsed = NULL;
    PTDNF_REPO_DATA pRepoTemp = NULL;
    int nCount = 0;

    if(!pRepoData || !pppszReposUsed)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepoTemp = pRepoData;
    while(pRepoTemp)
    {
        if(pRepoTemp->nEnabled)
        {
            ++nCount;
        }
        pRepoTemp = pRepoTemp->pNext;
    }
    if(nCount == 0)
    {
        dwError = ERROR_TDNF_NO_ENABLED_REPOS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  sizeof(char**) * (nCount + 1),
                  (void**)&ppszReposUsed);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepoTemp = pRepoData;
    while(pRepoTemp)
    {
        if(pRepoTemp->nEnabled)
        {
            dwError = TDNFAllocateString(
                          pRepoTemp->pszId,
                          &ppszReposUsed[--nCount]);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pRepoTemp = pRepoTemp->pNext;
    }

    *pppszReposUsed = ppszReposUsed;
cleanup:
    return dwError;

error:
    if(!pppszReposUsed)
    {
        *pppszReposUsed = NULL;
    }
    TDNF_SAFE_FREE_STRINGARRAY(ppszReposUsed);
    goto cleanup;
}

void
TDNFFreeCleanInfo(
    PTDNF_CLEAN_INFO pCleanInfo
    )
{
    if(pCleanInfo)
    {
        TDNF_SAFE_FREE_STRINGARRAY(pCleanInfo->ppszReposUsed);
        TDNFFreeMemory(pCleanInfo);
    }
}
