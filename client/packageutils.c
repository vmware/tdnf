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
TDNFMatchForReinstall(
    PSolvSack pSack,
    const char* pszName,
    Queue* pQueueGoal
    )
{
    uint32_t dwError = 0;
    Id  dwInstalledId = 0;
    Id  dwAvailableId = 0;
    char* pszNevr = NULL;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(
                  pSack,
                  pszName,
                  pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError =  SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPkgNevrFromId(pSack, dwInstalledId, &pszNevr);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreatePackageList(&pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszNevr,
                  pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageId(pAvailabePkgList,
                               0,
                               &dwAvailableId);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_push(pQueueGoal, dwAvailableId);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszNevr);
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
    int dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

    if(!ppPkgInfo || !pdwCount || !pSack || !pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPackageListSize(pPkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

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

    for (dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        pPkgInfo = &pPkgInfos[dwPkgIndex];

        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack, dwPkgId, &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgArchFromId(pSack, dwPkgId, &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgVersionFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        if(nDetail == DETAIL_INFO)
        {
            dwError = SolvGetPkgInstallSizeFromId(
                          pSack,
                          dwPkgId,
                          &pPkgInfo->dwInstallSizeBytes);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFUtilsFormatSize(
                          pPkgInfo->dwInstallSizeBytes,
                          &pPkgInfo->pszFormattedSize);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgSummaryFromId(
                          pSack,
                          dwPkgId,
                          &pPkgInfo->pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgUrlFromId(pSack, dwPkgId, &pPkgInfo->pszURL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgLicenseFromId(
                          pSack,
                          dwPkgId,
                          &pPkgInfo->pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgDescriptionFromId(
                          pSack,
                          dwPkgId,
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
        TDNFFreePackageInfoArray(pPkgInfos, dwCount);
    }
    goto cleanup;
}

uint32_t
TDNFAppendPackages(
    PTDNF_PKG_INFO* ppDest,
    PTDNF_PKG_INFO  pSource
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

uint32_t
TDNFPackageGetDowngrade(
    Id          dwInstalled,
    PSolvSack   pSack,
    PSolvPackageList pAvailabePkgList,
    Id*         pdwDowngradePkgId
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    int dwEvrCompare = 0;
    Id dwAvailableId = 0;
    Id dwDownGradeId = 0;
    uint32_t dwCount = 0;

    if(!pSack || !pdwDowngradePkgId || !pAvailabePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);;
    }

    dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId( pAvailabePkgList,
                                    dwPkgIndex,
                                    &dwAvailableId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvCmpEvr(
                      pSack,
                      dwAvailableId,
                      dwInstalled,
                      &dwEvrCompare);
        BAIL_ON_TDNF_ERROR(dwError);

        if(dwEvrCompare < 0)
        {
            if(dwDownGradeId == 0)
            {
                dwDownGradeId = dwAvailableId;
            }
            else
            {
                dwError = SolvCmpEvr(pSack,
                              dwAvailableId,
                              dwDownGradeId,
                              &dwEvrCompare);
                if(dwError == 0 && dwEvrCompare > 0)
                {
                    dwDownGradeId = dwAvailableId;
                }
            }
        }
    }

    if(dwDownGradeId == 0)
    {
        dwError = ERROR_TDNF_NO_DOWNGRADE_PATH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pdwDowngradePkgId = dwDownGradeId;
cleanup:

    return dwError;
error:
    if(pdwDowngradePkgId)
    {
        *pdwDowngradePkgId = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetGlobPackages(
    PSolvSack pSack,
    char* pszPkgGlob,
    Queue* pQueueGoal
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    PSolvPackageList pSolvPkgList = NULL;

    if(!pSack || IsNullOrEmptyString(pszPkgGlob) || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszPkgGlob,
                  pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pSolvPkgList->queuePackages.count > 0)
    {
        for(dwPkgIndex = 0; 
            dwPkgIndex < pSolvPkgList->queuePackages.count;
            dwPkgIndex++)
        {
            queue_push(pQueueGoal, 
                       pSolvPkgList->queuePackages.elements[dwPkgIndex]);
        }
    }

cleanup:
    if(pSolvPkgList)
    {
        SolvFreePackageList(pSolvPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFAddPackagesForErase(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id dwInstalledId = 0;
    int dwPkgIndex = 0;
    uint32_t dwCount = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(
                      pInstalledPkgList,
                      dwPkgIndex,
                      &dwInstalledId);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push(pQueueGoal, dwInstalledId);
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
TDNFAddPackagesForInstall(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwEvrCompare = 0;
    Id dwInstalledId = 0;
    Id dwHighestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pSack || !pQueueGoal ||IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);

    SolvFindHighestAvailable(pSack, pszPkgName, &dwHighestAvailable);
    if(dwHighestAvailable != 0)
    {
        if(dwInstalledId != 0)
        {
            dwError = SolvCmpEvr(pSack,
                         dwHighestAvailable,
                         dwInstalledId,
                         &dwEvrCompare);
            if(dwError == 0 && dwEvrCompare > 0)
            {
                queue_push(pQueueGoal, dwHighestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_ALREADY_INSTALLED;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            queue_push(pQueueGoal, dwHighestAvailable);
        }
    }
    else
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
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
TDNFVerifyUpgradePackage(
    PSolvSack pSack,
    Id dwPkg,
    uint32_t* dwUpgradePackage
    )
{

    uint32_t dwError = 0;
    char* pszName = NULL;
    Id  dwInstalledId = 0;
    int dwEvrCompare = 0;

    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgNameFromId(pSack, dwPkg, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindHightestInstalled(
                  pSack,
                  pszName,
                  &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCmpEvr(pSack, dwPkg, dwInstalledId, &dwEvrCompare);
    if(dwError == 0 && dwEvrCompare > 0)
    {
        *dwUpgradePackage = 1;
    }
    else
    {
        *dwUpgradePackage = 0;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_MATCH || dwError == ERROR_TDNF_NO_DATA)
    {
        *dwUpgradePackage = 1;
        dwError = 0;
    }
    goto cleanup;
}
uint32_t
TDNFAddPackagesForUpgrade(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id dwHighestAvailable = 0;
    uint32_t  dwUpgradePackage = 0;

    if(!pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindHighestAvailable(pSack,
                  pszPkgName,
                  &dwHighestAvailable);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFVerifyUpgradePackage(
                  pSack,
                  dwHighestAvailable,
                  &dwUpgradePackage);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwUpgradePackage == 1)
    {
        queue_push(pQueueGoal, dwHighestAvailable);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}


uint32_t
TDNFAddPackagesForDowngrade(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pAvailabePkgList = NULL;
    Id dwInstalledId = 0;
    Id dwAvailableId = 0;
    Id dwDownGradeId = 0;
    char* pszName = NULL;

    if(!pSack || !pQueueGoal || !pszPkgName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(pSack,
                  pszPkgName,
                  pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageId(pAvailabePkgList, 0, &dwAvailableId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPkgNameFromId(pSack, dwAvailableId, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindLowestInstalled(
                  pSack,
                  pszName,
                  &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPackageGetDowngrade(
                  dwInstalledId,
                  pSack,
                  pAvailabePkgList,
                  &dwDownGradeId);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_push(pQueueGoal, dwDownGradeId);
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
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
    int dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!ppPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPackageListSize(pPkgList, &dwCount);
    if(dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack, dwPkgId, &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgVersionFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgArchFromId(pSack, dwPkgId, &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgSummaryFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgLocationFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszLocation);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgInstallSizeFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->dwInstallSizeBytes);

        BAIL_ON_TDNF_ERROR(dwError);

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
