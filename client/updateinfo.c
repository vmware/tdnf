/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : updateinfo.c
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
    int nPkgCount = 0;
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

    dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgList, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    nPkgCount = hy_packagelist_count(hPkgList);
    if(nPkgCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                   nTypeCount + 1,
                   sizeof(TDNF_UPDATEINFO_SUMMARY),
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
                      1,
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
    if(pPkg)
    {
        TDNFFreeUpdateInfoPackages(pPkg);
    }
    if(pPkgs)
    {
        TDNFFreeUpdateInfoPackages(pPkgs);
    }

    goto cleanup;
}

uint32_t
TDNFGetUpdateInfoPackages2(
    HyAdvisory hAdv,
    PTDNF_UPDATEINFO_PKG2* ppPkgs
    )
{
    uint32_t dwError = 0;
    int nCount = 0;
    int iPkg = 0;
    HyAdvisoryPkgList hAdvPkgList = NULL;
    HyAdvisoryPkg hAdvPkg = NULL;

    PTDNF_UPDATEINFO_PKG2 pPkgs = NULL;
    PTDNF_UPDATEINFO_PKG2 pPkg = NULL;
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
                      1,
                      sizeof(TDNF_UPDATEINFO_PKG2),
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

        pPkg->reboot_suggested = hy_advisory_get_reboot_suggested(hAdv);

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
    if(pPkg)
    {
        TDNFFreeUpdateInfoPackages2(pPkg);
    }
    if(pPkgs)
    {
        TDNFFreeUpdateInfoPackages2(pPkgs);
    }

    goto cleanup;
}
