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
TDNFGetInstalled(
    PSolvSack hSack,
    PSolvPackageList* phPkgList
    )
{
    return 1;
}

uint32_t
TDNFMatchForReinstall(
    PTDNF pTdnf,
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    char** ppszPackages = NULL;
    int nCmdIndex = 1;
    int pkgIndex = 0;
    int evrCompare = 0;
    Id installedId = 0;
    Id availableId = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pTdnf || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    pInstalledPkgList = SolvCreatePackageList();
    if(!pInstalledPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pAvailabePkgList = SolvCreatePackageList();
    if(!pAvailabePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    ppszPackages = pTdnf->pArgs->ppszCmds;
    for(; nCmdIndex < pTdnf->pArgs->nCmdCount; nCmdIndex++)
    {
        SolvEmptyPackageList(pInstalledPkgList);
        SolvEmptyPackageList(pAvailabePkgList);
        dwError = SolvFindInstalledPkgByName(pTdnf->pSack, ppszPackages[nCmdIndex], pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = SolvFindAvailablePkgByName(pTdnf->pSack, ppszPackages[nCmdIndex], pAvailabePkgList);
        BAIL_ON_TDNF_ERROR(dwError);
        SolvGetPackageId(pInstalledPkgList, 0, &installedId);
        for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pAvailabePkgList); pkgIndex++)
        {
            SolvGetPackageId(pAvailabePkgList, 0, &availableId);
            dwError = SolvCmpEvr(pTdnf->pSack, installedId, availableId, &evrCompare);
            if(dwError == 0 && evrCompare == 0)
            {
                queue_push(&pQuery->result, availableId);
                break;
            }
        }

    }
cleanup:

    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
    }
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFMatchForDowngradeAll(
    PTDNF pTdnf,
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    int pkgIndexInstalled = 0;
    Id installedId = 0;
    Id availableId = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pInstalledPkgList = SolvCreatePackageList();
    if(!pInstalledPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindAllInstalled(pTdnf->pSack, pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);


    for(; pkgIndexInstalled < SolvGetPackageListSize(pInstalledPkgList); pkgIndexInstalled++)
    {
        SolvGetPackageId(pInstalledPkgList, pkgIndexInstalled, &installedId);
        dwError =  SolvFindDowngradeCandidate(pTdnf->pSack, installedId, &availableId);
        if(dwError == 0)
        {
            queue_push(&pQuery->result, availableId);
        }

    }
cleanup:
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFMatchForDowngrade(
    PTDNF pTdnf,
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    char** ppszPackages = NULL;
    int nCmdIndex = 1;
    int pkgIndex = 0;
    int evrCompare = 0;
    Id installedId = 0;
    Id availableId = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pTdnf || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    pInstalledPkgList = SolvCreatePackageList();
    if(!pInstalledPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pAvailabePkgList = SolvCreatePackageList();
    if(!pAvailabePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszPackages = pTdnf->pArgs->ppszCmds;
    for(; nCmdIndex < pTdnf->pArgs->nCmdCount; nCmdIndex++)
    {
        SolvEmptyPackageList(pInstalledPkgList);
        SolvEmptyPackageList(pAvailabePkgList);
        dwError = SolvFindAvailablePkgByName(pTdnf->pSack, ppszPackages[nCmdIndex], pAvailabePkgList);

        SolvGetPackageId(pAvailabePkgList, pkgIndex, &availableId);
        if(dwError)
            continue;

        dwError = SolvFindInstalledPkgByName(pTdnf->pSack, SolvGetPkgNameFromId(pTdnf->pSack, availableId), pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        SolvGetPackageId(pInstalledPkgList, 0, &installedId);
        for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pAvailabePkgList); pkgIndex++)
        {
            SolvGetPackageId(pAvailabePkgList, pkgIndex, &availableId);
            dwError = SolvCmpEvr(pTdnf->pSack, installedId, availableId, &evrCompare);
            if(dwError == 0 && evrCompare > 0)
            {
                queue_push(&pQuery->result, availableId);
                break;
            }
        }

    }
cleanup:

    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
    }
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPopulatePkgInfos(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfos
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int pkgIndex = 0;
    Id  pkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!ppPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = SolvGetPackageListSize(pPkgList);
    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pkgIndex = 0; pkgIndex < dwCount; pkgIndex++)
    {
        dwError = SolvGetPackageId(pPkgList, pkgIndex, &pkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      SolvGetPkgNameFromId(pSack, pkgId),
                      &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgVersionFromId(pSack, pkgId),
                    &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgReleaseFromId(pSack, pkgId),
                    &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);


        dwError = TDNFSafeAllocateString(
                      SolvGetPkgArchFromId(pSack, pkgId),
                      &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      SolvGetPkgRepoNameFromId(pSack, pkgId),
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      SolvGetPkgSummaryFromId(pSack, pkgId),
                      &pPkgInfo->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      SolvGetPkgLocationFromId(pSack, pkgId),
                      &pPkgInfo->pszLocation);
        BAIL_ON_TDNF_ERROR(dwError);

        pPkgInfo->dwInstallSizeBytes = SolvGetPkgInstallSizeFromId(pSack, pkgId);

        dwError = TDNFUtilsFormatSize(
                        pPkgInfo->dwInstallSizeBytes,
                        &pPkgInfo->pszFormattedSize);
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

uint32_t
TDNFPopulatePkgInfoArray(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    TDNF_PKG_DETAIL nDetail,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int pkgIndex = 0;
    Id  pkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

    if(!ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = SolvGetPackageListSize(pPkgList);

    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                dwCount,
                sizeof(TDNF_PKG_INFO),
                (void**)&pPkgInfos);
    BAIL_ON_TDNF_ERROR(dwError);

    for (pkgIndex = 0; pkgIndex < dwCount; pkgIndex++)
    {
        dwError = SolvGetPackageId(pPkgList, pkgIndex, &pkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        PTDNF_PKG_INFO pPkgInfo = &pPkgInfos[pkgIndex];

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgNameFromId(pSack, pkgId),
                    &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);


        dwError = TDNFSafeAllocateString(
                    SolvGetPkgArchFromId(pSack, pkgId),
                    &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgVersionFromId(pSack, pkgId),
                    &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgReleaseFromId(pSack, pkgId),
                    &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    SolvGetPkgRepoNameFromId(pSack, pkgId),
                    &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);
        if(nDetail == DETAIL_INFO)
        {
            pPkgInfo->dwInstallSizeBytes = SolvGetPkgInstallSizeFromId(pSack, pkgId);

            dwError = TDNFUtilsFormatSize(
                        pPkgInfo->dwInstallSizeBytes,
                        &pPkgInfo->pszFormattedSize);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        SolvGetPkgSummaryFromId(pSack, pkgId),
                        &pPkgInfo->pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        SolvGetPkgUrlFromId(pSack, pkgId),
                        &pPkgInfo->pszURL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        SolvGetPkgLicenseFromId(pSack, pkgId),
                        &pPkgInfo->pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        SolvGetPkgDescriptionFromId(pSack, pkgId),
                        &pPkgInfo->pszDescription);
            BAIL_ON_TDNF_ERROR(dwError);

        }
    }

    *pdwCount = dwCount;
    *ppPkgInfo = pPkgInfos;

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
    if(pPkgInfos)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    goto cleanup;
}

uint32_t
TDNFAppendPackages(
    PTDNF_PKG_INFO* ppDest,
    PTDNF_PKG_INFO pSource
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pDest = NULL;

    if(!ppDest || !pSource)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pDest = *ppDest;
    if(!pDest)
    {
        *ppDest = pSource;
    }
    else
    {
        while(pDest->pNext)
        {
            pDest = pDest->pNext;
        }
        pDest->pNext = pSource;
    }

cleanup:
    return dwError;

error:
    if(ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}


