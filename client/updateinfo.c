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
TDNFCheckSecurityOption(
    PTDNF pTdnf,
    uint32_t *pdwSecurity
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwSecurity = 0;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //There could be overrides to enable/disable
    //repo such as cmdline args, api overrides
    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "security"))
        {
            dwSecurity = 1;
            break;
         }
         pSetOpt = pSetOpt->pNext;
    }

    *pdwSecurity = dwSecurity;
cleanup:
    return dwError;

error:
    if(pdwSecurity)
    {
        *pdwSecurity = 0;
    }
    goto cleanup;
}

uint32_t
TDNFCheckSeverityOption(
    PTDNF pTdnf,
    char **ppszSeverity
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    char* pszSeverity = NULL;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //There could be overrides to enable/disable
    //repo such as cmdline args, api overrides
    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "sec-severity"))
        {
            dwError = TDNFAllocateString(pSetOpt->pszOptValue, &pszSeverity);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
         }
         pSetOpt = pSetOpt->pNext;
    }

    *ppszSeverity = pszSeverity;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    if(ppszSeverity)
    {
        *ppszSeverity = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFNumUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    if(!pInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(pInfo)
    {
        pPkg = pInfo->pPackages;
        while(pPkg)
        {
            dwCount++;
            pPkg = pPkg->pNext;
        }
        pInfo = pInfo->pNext;
    }
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
TDNFGetUpdatePkgs(
    PTDNF pTdnf,
    char*** pppszPkgs,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    char**   ppszPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    int nIndex = 0;
    char* pszPkgName = NULL;

    PTDNF_UPDATEINFO pUpdateInfo = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;

    if(!pTdnf || !pdwCount || !pppszPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFUpdateInfo(pTdnf, 0, 0, &pszPkgName, &pUpdateInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pInfo = pUpdateInfo;
    dwError = TDNFNumUpdatePkgs(pInfo, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwCount == 0)
    {
        goto cleanup;
    }

    dwError = TDNFAllocateMemory(
                  dwCount + 1,
                  sizeof(char*),
                  (void**)&ppszPkgs);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pInfo = pUpdateInfo; pInfo; pInfo = pInfo->pNext)
    {
        pPkg = pInfo->pPackages;
        while(pPkg)
        {
            dwError = TDNFAllocateString(
                          pPkg->pszName,
                          &ppszPkgs[nIndex++]);
            BAIL_ON_TDNF_ERROR(dwError);
            pPkg = pPkg->pNext;
        }

    }
    *pppszPkgs = ppszPkgs;
    *pdwCount  = dwCount;
cleanup:
    if(pUpdateInfo)
    {
        TDNFFreeUpdateInfo(pUpdateInfo);
    }
    return dwError;

error:
    if(pppszPkgs)
    {
        *pppszPkgs = NULL;
    }
    if(ppszPkgs)
    {
        TDNFFreeStringArray(ppszPkgs);
    }
    goto cleanup;
}
