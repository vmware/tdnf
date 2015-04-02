/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : client.c
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
TDNFApplyScopeFilter(
    HyQuery hQuery,
    TDNF_SCOPE nScope
    );

static
uint32_t
TDNFApplyPackageFilter(
    HyQuery hQuery,
    char** ppszPackageNameSpecs
    );

uint32_t
TDNFCountCommand(
    PTDNF pTdnf,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;

    if(!pTdnf || !pdwCount)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = hy_sack_count(pTdnf->hSack);
    *pdwCount = dwCount;
cleanup:
    return dwError;
error:
    if(pdwCount)
    {
      *pdwCount = 0;
    }
    goto cleanup;
}

uint32_t
TDNFCheckUpdates(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    return TDNFList(
               pTdnf,
               SCOPE_UPGRADES,
               ppszPackageNameSpecs,
               ppPkgInfo,
               pdwCount);
}

uint32_t
TDNFList(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;

    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int nIndex = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !ppszPackageNameSpecs || !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(hQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyPackageFilter(hQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
      dwError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwCount = hy_packagelist_count(hPkgList);
    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }
  
    dwError = TDNFAllocateMemory(
                sizeof(TDNF_PKG_INFO) * dwCount,
                (void**)&pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    for(nIndex = 0; (hPkg = hy_packagelist_get(hPkgList, nIndex)) != NULL; ++nIndex)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];
        dwError = TDNFSafeAllocateString(hy_package_get_name(hPkg), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_version(hPkg), &pPkg->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_release(hPkg), &pPkg->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);
    
        dwError = TDNFSafeAllocateString(hy_package_get_arch(hPkg), &pPkg->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_reponame(hPkg), &pPkg->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);
        //Should not free package as packagelist is freed below.
    }

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
  
    return dwError;
error:
    if(ppPkgInfo)
    {
      *ppPkgInfo = NULL;
    }
    if(pdwCount)
    {
      *pdwCount = 0;
    }
    TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    goto cleanup;
}

//Lists info on each installed package
//Returns a sum of installed size
uint32_t
TDNFInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;

    uint32_t dwError = 0;
    uint32_t dwCount = 0;

    int nIndex = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;
  
    if(!pTdnf || !pdwCount || !ppPkgInfo)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
      dwError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(hQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyPackageFilter(hQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
      dwError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwCount = hy_packagelist_count(hPkgList);

    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                sizeof(TDNF_PKG_INFO) * dwCount,
                (void**)&pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    for(nIndex = 0; (hPkg = hy_packagelist_get(hPkgList, nIndex)) != NULL; ++nIndex)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];

        pPkg->dwEpoch = hy_package_get_epoch(hPkg);
        pPkg->dwInstallSizeBytes = hy_package_get_installsize(hPkg);

        dwError = TDNFUtilsFormatSize(pPkg->dwInstallSizeBytes, &pPkg->pszFormattedSize);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_name(hPkg), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_arch(hPkg), &pPkg->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_version(hPkg), &pPkg->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_release(hPkg), &pPkg->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_reponame(hPkg), &pPkg->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_summary(hPkg), &pPkg->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_url(hPkg), &pPkg->pszURL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_license(hPkg), &pPkg->pszLicense);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(hy_package_get_description(hPkg), &pPkg->pszDescription);
        BAIL_ON_TDNF_ERROR(dwError);
        //Should not free package as packagelist is freed below.
    }

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    return dwError;

error:
    if(ppPkgInfo)
    {
        *ppPkgInfo = NULL;
    }
    if(pdwCount)
    {
        *pdwCount = 0;
    }  
    TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    goto cleanup;
}

uint32_t
TDNFApplyScopeFilter(
    HyQuery hQuery,
    TDNF_SCOPE nScope
    )
{
    uint32_t dwError = 0;

    if(!hQuery || nScope == SCOPE_NONE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    switch(nScope)
    {
        case SCOPE_INSTALLED:
            dwError = hy_query_filter(
                          hQuery,
                          HY_PKG_REPONAME,
                          HY_EQ,
                          HY_SYSTEM_REPO_NAME);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;

        case SCOPE_AVAILABLE:
            dwError = hy_query_filter(
                          hQuery,
                          HY_PKG_REPONAME,
                          HY_NEQ,
                          HY_SYSTEM_REPO_NAME);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
           
        case SCOPE_UPGRADES:
            hy_query_filter_upgrades(hQuery, 1);
            break;

        case SCOPE_RECENT:
            hy_query_filter_latest_per_arch(hQuery, 1);
            break;

        default:
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFApplyPackageFilter(
    HyQuery hQuery,
    char** ppszPackageNameSpecs
    )
{
    uint32_t dwError = 0;
    if(!hQuery || !ppszPackageNameSpecs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(*ppszPackageNameSpecs)
    {
        if(TDNFIsGlob(*ppszPackageNameSpecs))
        {
            hy_query_filter(hQuery, HY_PKG_NAME, HY_GLOB, *ppszPackageNameSpecs);
        }
        else
        {
            hy_query_filter(hQuery, HY_PKG_NAME, HY_EQ, *ppszPackageNameSpecs);
        }
        ++ppszPackageNameSpecs;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
