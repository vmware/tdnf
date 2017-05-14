/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
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
    uint32_t nCount = 0;
    int iAdv = 0;
    uint32_t dwPkgIndex = 0;
    uint32_t dwSize = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pUpdateAdvPkgList = NULL;
    Id dwAdvId = 0;
    Id dwPkgId = 0;
    uint32_t nType = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;
    const char *pszType = 0;

    if(!pTdnf || !pTdnf->pSack || !pTdnf->pSack->pPool ||
       !ppSummary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!ppszPackageNameSpecs)
    {
        dwError = SolvFindAllInstalled(pTdnf->pSack, &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = SolvFindInstalledPkgByMultipleNames(
                      pTdnf->pSack,
                      ppszPackageNameSpecs,
                      &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  UPDATE_ENHANCEMENT + 1,
                  sizeof(TDNF_UPDATEINFO_SUMMARY),
                  (void**)&pSummary);
    BAIL_ON_TDNF_ERROR(dwError);

    pSummary[UPDATE_UNKNOWN].nType = UPDATE_UNKNOWN;
    pSummary[UPDATE_SECURITY].nType = UPDATE_SECURITY;
    pSummary[UPDATE_BUGFIX].nType = UPDATE_BUGFIX;
    pSummary[UPDATE_ENHANCEMENT].nType = UPDATE_ENHANCEMENT;

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetUpdateAdvisories(
                      pTdnf->pSack,
                      dwPkgId,
                      &pUpdateAdvPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPackageListSize(pUpdateAdvPkgList, &nCount);
        BAIL_ON_TDNF_ERROR(dwError);

        for(iAdv = 0; iAdv < nCount; iAdv++)
        {
            dwError = SolvGetPackageId(pUpdateAdvPkgList, iAdv, &dwAdvId);
            BAIL_ON_TDNF_ERROR(dwError);

            pszType = pool_lookup_str(
                          pTdnf->pSack->pPool,
                          dwAdvId,
                          SOLVABLE_PATCHCATEGORY);
            nType = UPDATE_UNKNOWN;
            if (pszType == NULL)
                nType = UPDATE_UNKNOWN;
            else if (!strcmp (pszType, "bugfix"))
                nType = UPDATE_BUGFIX;
            else if (!strcmp (pszType, "enhancement"))
                nType = UPDATE_ENHANCEMENT;
            else if (!strcmp (pszType, "security"))
                nType = UPDATE_SECURITY;
            pSummary[nType].nCount++;
        }
        SolvFreePackageList(pUpdateAdvPkgList);
        pUpdateAdvPkgList = NULL;
    }
    *ppSummary = pSummary;

cleanup:
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    if(pUpdateAdvPkgList)
    {
        SolvFreePackageList(pUpdateAdvPkgList);
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
    PSolvSack pSack,
    Id dwPkgId,
    PTDNF_UPDATEINFO_PKG* ppUpdateInfoPkg
    )
{
    uint32_t dwError = 0;
    Dataiterator di = {0};
    PTDNF_UPDATEINFO_PKG pPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    const char* pszTemp = NULL;


    if(!pSack || !pSack->pPool || !ppUpdateInfoPkg)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dataiterator_init(&di, pSack->pPool, 0, dwPkgId, UPDATE_COLLECTION, 0, 0);
    while (dataiterator_step(&di))
    {
        dataiterator_setpos(&di);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_UPDATEINFO_PKG),
                      (void**)&pPkg);
        BAIL_ON_TDNF_ERROR(dwError);

        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_NAME);

        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszName);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_EVR);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszEVR);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_ARCH);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszArch);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_FILENAME);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszFileName);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        pPkg->pNext = pPkgs;
        pPkgs = pPkg;
        pPkg = NULL;


    }

    *ppUpdateInfoPkg = pPkgs;

cleanup:
    dataiterator_free(&di);
    return dwError;

error:
    if(ppUpdateInfoPkg)
    {
        *ppUpdateInfoPkg = NULL;
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
TDNFPopulateUpdateInfoOfOneAdvisory(
    PSolvSack pSack,
    Id dwAdvId,
    PTDNF_UPDATEINFO* ppInfo)
{
    uint32_t dwError = 0;

    time_t dwUpdated = 0;
    const char *pszType = 0;
    PTDNF_UPDATEINFO pInfo = NULL;
    const char* pszTemp = NULL;
    const int DATELEN = 200;
    char szDate[DATELEN];

    struct tm* pLocalTime = NULL;

    if(!pSack || !pSack->pPool || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_UPDATEINFO),
                  (void**)&pInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pszType = pool_lookup_str(pSack->pPool,
                              dwAdvId,
                              SOLVABLE_PATCHCATEGORY);
    pInfo->nType = UPDATE_UNKNOWN;
    if (pszType == NULL)
        pInfo->nType = UPDATE_UNKNOWN;
    else if (!strcmp (pszType, "bugfix"))
        pInfo->nType = UPDATE_BUGFIX;
    else if (!strcmp (pszType, "enhancement"))
        pInfo->nType = UPDATE_ENHANCEMENT;
    else if (!strcmp (pszType, "security"))
        pInfo->nType = UPDATE_SECURITY;

    pszTemp = pool_lookup_str(
                  pSack->pPool,
                  dwAdvId,
                  SOLVABLE_NAME);
    if(pszTemp)
    {
        dwError = TDNFAllocateString(pszTemp, &pInfo->pszID);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszTemp = pool_lookup_str(
                  pSack->pPool,
                  dwAdvId,
                  SOLVABLE_DESCRIPTION);
    if(pszTemp)
    {
        dwError = TDNFAllocateString(pszTemp, &pInfo->pszDescription);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwUpdated = pool_lookup_num(
                    pSack->pPool,
                    dwAdvId,
                    SOLVABLE_BUILDTIME,
                    0);
    if(dwUpdated > 0)
    {
        pLocalTime = localtime(&dwUpdated);
        if(!pLocalTime)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        memset(szDate, 0, DATELEN);
        dwError = strftime(szDate, DATELEN, "%c", pLocalTime);
        if(dwError == 0)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        dwError = TDNFAllocateString(szDate, &pInfo->pszDate);
        BAIL_ON_TDNF_ERROR(dwError);
    }


    dwError = TDNFGetUpdateInfoPackages(pSack, dwAdvId, &pInfo->pPackages);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppInfo = pInfo;

cleanup:
    return dwError;

error:
    if(ppInfo)
    {
        *ppInfo = NULL;
    }
    if(pInfo)
    {
        TDNFFreeUpdateInfo(pInfo);
    }
    goto cleanup;
}
