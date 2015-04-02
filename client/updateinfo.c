/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : updateinfo.c
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
TDNFGetUpdateInfoPackages(
    HyAdvisory hAdv,
    PTDNF_UPDATEINFO_PKG* ppUpdateInfoPkg
    );

static
void
TDNFFreeUpdateInfoReferences(
    PTDNF_UPDATEINFO_REF pRef
    );

static
void
TDNFFreeUpdateInfoPackages(
    PTDNF_UPDATEINFO_PKG pPkg
    );

//TODO: Refactor UpdateInfoSummary into one function
uint32_t
TDNFUpdateInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    TDNF_AVAIL nAvail,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO* ppUpdateInfo
    )
{
    uint32_t dwError = 0;

    int nCount = 0;
    int iPkg = 0;
    int iAdv = 0;

    PTDNF_UPDATEINFO pUpdateInfos = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;
    const char* pszTemp = NULL;

    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    HyAdvisoryList hAdvList = NULL;
    HyAdvisory hAdv = NULL;

    if(!pTdnf || !ppszPackageNameSpecs || !ppUpdateInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    FOR_PACKAGELIST(hPkg, hPkgList, iPkg)
    {
        hAdvList = hy_package_get_advisories(hPkg, HY_GT);
        if(!hAdvList)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        nCount = hy_advisorylist_count(hAdvList);
        for(iAdv = 0; iAdv < nCount; iAdv++)
        {
            dwError = TDNFAllocateMemory(
                          sizeof(TDNF_UPDATEINFO),
                          (void**)&pInfo);
            BAIL_ON_TDNF_ERROR(dwError);

            hAdv = hy_advisorylist_get_clone(hAdvList, iAdv);
            if(!hAdv)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }

            pInfo->nType = hy_advisory_get_type(hAdv);
            pszTemp = hy_advisory_get_id(hAdv);
            if(pszTemp)
            {
                dwError = TDNFAllocateString(pszTemp, &pInfo->pszID);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFGetUpdateInfoPackages(hAdv, &pInfo->pPackages);
            BAIL_ON_TDNF_ERROR(dwError);

            hy_advisory_free(hAdv);
            hAdv = NULL;

            pInfo->pNext = pUpdateInfos;
            pUpdateInfos = pInfo;
            pInfo = NULL;
        }
        hy_advisorylist_free(hAdvList);
        hAdvList = NULL;
    }

    *ppUpdateInfo = pUpdateInfos;

cleanup:
    if(hAdv)
    {
        hy_advisory_free(hAdv);
    }
    if(hAdvList)
    {
        hy_advisorylist_free(hAdvList);
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(ppUpdateInfo)
    {
        *ppUpdateInfo = NULL;
    }
    if(pUpdateInfos)
    {
        TDNFFreeUpdateInfo(pUpdateInfos);
    }
    if(pInfo)
    {
        TDNFFreeUpdateInfo(pInfo);
    }
    goto cleanup;
}

uint32_t
TDNFUpdateInfoSummary(
    PTDNF pTdnf,
    TDNF_AVAIL nAvail,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO_SUMMARY* ppSummary
    )
{
    uint32_t dwError = 0;
    int nCount = 0;
    int iPkg = 0;
    int iAdv = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;

    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    HyAdvisoryList hAdvList = NULL;
    HyAdvisory hAdv = NULL;
    HyAdvisoryType nType = HY_ADVISORY_UNKNOWN;
    int nTypeCount = HY_ADVISORY_ENHANCEMENT;

    if(!pTdnf || !ppszPackageNameSpecs || !ppSummary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                   sizeof(TDNF_UPDATEINFO_SUMMARY) * (nTypeCount+1),
                   (void**)&pSummary);
    BAIL_ON_TDNF_ERROR(dwError);
    
    pSummary[HY_ADVISORY_UNKNOWN].nType = UPDATE_UNKNOWN;
    pSummary[HY_ADVISORY_SECURITY].nType = UPDATE_SECURITY;
    pSummary[HY_ADVISORY_BUGFIX].nType = UPDATE_BUGFIX;
    pSummary[HY_ADVISORY_ENHANCEMENT].nType = UPDATE_ENHANCEMENT;

    FOR_PACKAGELIST(hPkg, hPkgList, iPkg)
    {
        hAdvList = hy_package_get_advisories(hPkg, HY_GT);
        if(!hAdvList)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        nCount = hy_advisorylist_count(hAdvList);
        for(iAdv = 0; iAdv < nCount; iAdv++)
        {
            hAdv = hy_advisorylist_get_clone(hAdvList, iAdv);
            if(!hAdv)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            nType = hy_advisory_get_type(hAdv);
            if(nType < HY_ADVISORY_UNKNOWN || nType > HY_ADVISORY_ENHANCEMENT)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            pSummary[nType].nCount++;
            hy_advisory_free(hAdv);
            hAdv = NULL;
        }
        hy_advisorylist_free(hAdvList);
        hAdvList = NULL;
    }
    *ppSummary = pSummary;

cleanup:
    if(hAdv)
    {
        hy_advisory_free(hAdv);
    }
    if(hAdvList)
    {
        hy_advisorylist_free(hAdvList);
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(ppSummary)
    {
        *ppSummary = NULL;
    }
    TDNFFreeUpdateInfoSummary(pSummary);
    goto cleanup;
}

void
TDNFFreeUpdateInfoSummary(
    PTDNF_UPDATEINFO_SUMMARY pSummary
    )
{
    if(pSummary)
    {
        TDNFFreeMemory(pSummary);
    }
}

void
TDNFFreeUpdateInfo(
    PTDNF_UPDATEINFO pUpdateInfo
    )
{
    if(pUpdateInfo)
    {
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszID);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDate);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDescription);

        TDNFFreeUpdateInfoReferences(pUpdateInfo->pReferences);
        TDNFFreeUpdateInfoPackages(pUpdateInfo->pPackages);
    }
}

void
TDNFFreeUpdateInfoReferences(
    PTDNF_UPDATEINFO_REF pRef
    )
{
    if(pRef)
    {
        TDNF_SAFE_FREE_MEMORY(pRef->pszID);
        TDNF_SAFE_FREE_MEMORY(pRef->pszLink);
        TDNF_SAFE_FREE_MEMORY(pRef->pszTitle);
        TDNF_SAFE_FREE_MEMORY(pRef->pszType);
    }
}

uint32_t
TDNFGetUpdateInfoPackages(
    HyAdvisory hAdv,
    PTDNF_UPDATEINFO_PKG* ppPkgs
    )
{
    uint32_t dwError = 0;
    int nCount = 0;
    int iPkg = 0;
    HyAdvisoryPkgList hAdvPkgList = NULL;
    HyAdvisoryPkg hAdvPkg = NULL;

    PTDNF_UPDATEINFO_PKG pPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    const char* pszTemp = NULL;


    if(!hAdv || !ppPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hAdvPkgList = hy_advisory_get_packages(hAdv);
    if(!hAdvPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nCount = hy_advisorypkglist_count(hAdvPkgList);
    for(iPkg = 0; iPkg < nCount; iPkg++)
    {
        hAdvPkg = hy_advisorypkglist_get_clone(hAdvPkgList, iPkg);
        if(!hAdvPkg)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFAllocateMemory(
                      sizeof(TDNF_UPDATEINFO_PKG),
                      (void**)&pPkg);
        BAIL_ON_TDNF_ERROR(dwError);

        pszTemp = hy_advisorypkg_get_string(hAdvPkg, HY_ADVISORYPKG_NAME);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszName);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = hy_advisorypkg_get_string(hAdvPkg, HY_ADVISORYPKG_EVR);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszEVR);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = hy_advisorypkg_get_string(hAdvPkg, HY_ADVISORYPKG_ARCH);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszArch);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = hy_advisorypkg_get_string(hAdvPkg, HY_ADVISORYPKG_FILENAME);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszFileName);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        hy_advisorypkg_free(hAdvPkg);
        hAdvPkg = NULL;

        pPkg->pNext = pPkgs;
        pPkgs = pPkg;
        pPkg = NULL;
    }

    *ppPkgs = pPkgs;

cleanup:
    if(hAdvPkg)
    {
        hy_advisorypkg_free(hAdvPkg);
    }
    if(hAdvPkgList)
    {
        hy_advisorypkglist_free(hAdvPkgList);
    }
    return dwError;

error:
    if(ppPkgs)
    {
        *ppPkgs = NULL;
    }
    if(pPkgs)
    {
        TDNFFreeUpdateInfoPackages(pPkgs);
    }

    goto cleanup;
}

void
TDNFFreeUpdateInfoPackages(
    PTDNF_UPDATEINFO_PKG pPkgs
    )
{
    PTDNF_UPDATEINFO_PKG pTemp = NULL;
    while(pPkgs)
    {
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszFileName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszEVR);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszArch);

        pTemp = pPkgs;
        pPkgs = pPkgs->pNext;
        
        TDNF_SAFE_FREE_MEMORY(pTemp);
    }
}
