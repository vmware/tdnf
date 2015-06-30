/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : packageutils.c
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
TDNFGetPackageVersion(
    HyPackage hPkg,
    char** ppszVersion
    );

uint32_t
TDNFGetPackageRelease(
    HyPackage hPkg,
    char** ppszRelease
    );

uint32_t
TDNFFindAvailablePkgByPkg(
    HySack hSack,
    HyPackage hPkgToFind,
    HyPackage* phPkg
    )
{
    uint32_t dwError = 0;
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;
    char* pszHyNevra = NULL;

    if(!hSack || !hPkgToFind || !phPkg)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    hQuery = hy_query_create(hSack);
    if(!hQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_REPONAME,
                  HY_NEQ,
                  HY_SYSTEM_REPO_NAME);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    pszHyNevra = hy_package_get_nevra(hPkgToFind);
    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_NEVRA,
                  HY_EQ,
                  pszHyNevra);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(hy_packagelist_count(hPkgList) == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hPkg = hy_packagelist_get_clone(hPkgList, 0);
    if(!hPkg)
    {
        dwError = hy_get_errno();
        if(dwError == 0)
        {
            dwError = HY_E_FAILED;
        }
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    *phPkg = hPkg;

cleanup:
    if(pszHyNevra)
    {
        hy_free(pszHyNevra);
    }
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
    if(phPkg)
    {
        *phPkg = NULL;
    }
    if(hPkg)
    {
        hy_package_free(hPkg);
    }
    goto cleanup;
}

uint32_t
TDNFFindInstalledPkgByName(
    HySack hSack,
    const char* pszName,
    HyPackage* phPkg
    )
{
    uint32_t dwError = 0;
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;

    if(!hSack || IsNullOrEmptyString(pszName) || !phPkg)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    hQuery = hy_query_create(hSack);
    if(!hQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_REPONAME,
                  HY_EQ,
                  HY_SYSTEM_REPO_NAME);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_NAME,
                  HY_EQ,
                  pszName);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(hy_packagelist_count(hPkgList) == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hPkg = hy_packagelist_get_clone(hPkgList, 0);
    if(!hPkg)
    {
        dwError = hy_get_errno();
        if(dwError == 0)
        {
            dwError = HY_E_FAILED;
        }
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    *phPkg = hPkg;

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
    if(phPkg)
    {
        *phPkg = NULL;
    }
    if(hPkg)
    {
        hy_package_free(hPkg);
    }
    goto cleanup;
}

uint32_t
TDNFGetInstalled(
    HySack hSack,
    HyPackageList* phPkgList
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgList = NULL;
    HyQuery hQuery = NULL;

    if(!hSack || !phPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(hSack);
    if(!hQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_REPONAME,
                  HY_EQ,
                  HY_SYSTEM_REPO_NAME);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *phPkgList = hPkgList;
cleanup:
    return dwError;

error:
    if(phPkgList)
    {
        *phPkgList = NULL;
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    goto cleanup;
}

uint32_t
TDNFMatchForReinstall(
    HySack hSack,
    const char* pszName,
    HyPackageList* phPkgList
    )
{
    uint32_t dwError = 0;
    HyPackage hPkgAvailable = NULL;
    HyPackage hPkgInstalled = NULL;
    HyPackageList hPkgList = NULL;

    if(!hSack || IsNullOrEmptyString(pszName) || !phPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFFindInstalledPkgByName(hSack, pszName, &hPkgInstalled);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFFindAvailablePkgByPkg(hSack, hPkgInstalled, &hPkgAvailable);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgList = hy_packagelist_create();
    if(!hPkgList)
    {
        dwError = hy_get_errno();
        if(dwError == 0)
        {
            dwError = HY_E_FAILED;
        }
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    hy_packagelist_push(hPkgList, hPkgAvailable);

    hPkgAvailable = NULL;
    *phPkgList = hPkgList;
cleanup:
    return dwError;

error:
    if(phPkgList)
    {
        *phPkgList = NULL;
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hPkgAvailable)
    {
        hy_package_free(hPkgAvailable);
    }
    goto cleanup;
}

uint32_t
TDNFPopulatePkgInfos(
    HyPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfos
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int nIndex = 0;

    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    HyPackage hPkg = NULL;

    if(!hPkgList || !ppPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = hy_packagelist_count(hPkgList);
    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    FOR_PACKAGELIST(hPkg, hPkgList, nIndex)
    {
        dwError = TDNFAllocateMemory(
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      hy_package_get_name(hPkg),
                      &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFGetPackageVersion(hPkg, &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFGetPackageRelease(hPkg, &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);


        dwError = TDNFSafeAllocateString(
                      hy_package_get_arch(hPkg),
                      &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      hy_package_get_reponame(hPkg),
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      hy_package_get_summary(hPkg),
                      &pPkgInfo->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        pPkgInfo->pNext = pPkgInfos;
        pPkgInfos = pPkgInfo;
        pPkgInfo = NULL;
    }

    *ppPkgInfos = pPkgInfos;

cleanup:

    return dwError;

error:

    if(ppPkgInfos)
    {
        *ppPkgInfos = NULL;
    }
    if (pPkgInfos)
    {
        TDNFFreePackageInfo(pPkgInfos);
    }
    if (pPkgInfo)
    {
        TDNFFreePackageInfo(pPkgInfo);
    }
    goto cleanup;
}

//
uint32_t
TDNFPopulatePkgInfoArray(
    HyPackageList hPkgList,
    TDNF_PKG_DETAIL nDetail,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int nIndex = 0;
    HyPackage hPkg = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!hPkgList || !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(nDetail != DETAIL_INFO && nDetail != DETAIL_LIST)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
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
        if(nDetail == DETAIL_INFO)
        {
            pPkg->dwEpoch = hy_package_get_epoch(hPkg);
            pPkg->dwInstallSizeBytes = hy_package_get_installsize(hPkg);

            dwError = TDNFUtilsFormatSize(
                          pPkg->dwInstallSizeBytes,
                          &pPkg->pszFormattedSize);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_name(hPkg),
                          &pPkg->pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_arch(hPkg),
                          &pPkg->pszArch);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFGetPackageVersion(hPkg, &pPkg->pszVersion);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFGetPackageRelease(hPkg, &pPkg->pszRelease);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_reponame(hPkg),
                          &pPkg->pszRepoName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_summary(hPkg),
                          &pPkg->pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_url(hPkg),
                          &pPkg->pszURL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_license(hPkg),
                          &pPkg->pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_description(hPkg),
                          &pPkg->pszDescription);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if(nDetail == DETAIL_LIST)
        {
            dwError = TDNFSafeAllocateString(
                          hy_package_get_name(hPkg),
                          &pPkg->pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_arch(hPkg),
                          &pPkg->pszArch);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFGetPackageVersion(hPkg, &pPkg->pszVersion);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFGetPackageRelease(hPkg, &pPkg->pszRelease);

            dwError = TDNFSafeAllocateString(
                          hy_package_get_reponame(hPkg),
                          &pPkg->pszRepoName);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        //Should not free package as packagelist is freed below.
    }

    *pdwCount = dwCount;
    *ppPkgInfo = pPkgInfo;

cleanup:
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
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    goto cleanup;
}

uint32_t
TDNFGetPackageVersion(
    HyPackage hPkg,
    char** ppszVersion
    )
{
    uint32_t dwError = 0;
    char* pszVersion = NULL;
    char* pszHyVersion = NULL;

    if(!hPkg || !ppszVersion)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszHyVersion = hy_package_get_version(hPkg);

    dwError = TDNFSafeAllocateString(
                  pszHyVersion,
                  &pszVersion);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszVersion = pszVersion;

cleanup:
    if(pszHyVersion)
    {
        hy_free(pszHyVersion);
    }
    return dwError;
error:
    if(ppszVersion)
    {
        *ppszVersion = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFGetPackageRelease(
    HyPackage hPkg,
    char** ppszRelease
    )
{
    uint32_t dwError = 0;
    char* pszRelease = NULL;
    char* pszHyRelease = NULL;

    if(!hPkg || !ppszRelease)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszHyRelease = hy_package_get_release(hPkg);

    dwError = TDNFSafeAllocateString(
                  pszHyRelease,
                  &pszRelease);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszRelease = pszRelease;

cleanup:
    if(pszHyRelease)
    {
        hy_free(pszHyRelease);
    }
    return dwError;

error:
    if(ppszRelease)
    {
        *ppszRelease = NULL;
    }
    goto cleanup;
}
