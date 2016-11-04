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
    PSolvSack   pSack,
    const char* pszName,
    Queue*      pQueueGoal
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int dwPkgIndex = 0;
    int dwEvrCompare = 0;
    int pkgNotFound = 0;
    Id installedId = 0;
    Id availableId = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreatePackageList(&pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pSack, pszName, pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);
    dwError = SolvFindAvailablePkgByName(pSack, pszName, pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError =  SolvGetPackageId(pInstalledPkgList, 0, &installedId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pAvailabePkgList, 0, &availableId);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = SolvCmpEvr(pSack, installedId, availableId, &dwEvrCompare);
        if(dwError == 0 && dwEvrCompare == 0)
        {
            queue_push(pQueueGoal, availableId);
            pkgNotFound = 1;
            break;
        }
    }

    if(pkgNotFound == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
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
    Id  pkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;
    const char* pszName = NULL;
    const char* pszArch = NULL;
    const char* pszSummary = NULL;
    const char* pszLicense = NULL;
    const char* pszDescription = NULL;
    const char* pszUrl = NULL;
    const char* pszRepoName = NULL;
    const char* pszVersion = NULL;
    const char* pszRelease = NULL;

    if(!ppPkgInfo || !pdwCount)
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
        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &pkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        PTDNF_PKG_INFO pPkgInfo = &pPkgInfos[dwPkgIndex];

        dwError = SolvGetPkgNameFromId(pSack, pkgId, &pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszName,
                    &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);


        dwError = SolvGetPkgArchFromId(pSack, pkgId, &pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszArch,
                    &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgVersionFromId(pSack, pkgId, &pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszVersion,
                    &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(pSack, pkgId, &pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszRelease,
                    &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(pSack, pkgId, &pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszRepoName,
                    &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);
        if(nDetail == DETAIL_INFO)
        {
            dwError = SolvGetPkgInstallSizeFromId(pSack, 
                        pkgId, &pPkgInfo->dwInstallSizeBytes);

            BAIL_ON_TDNF_ERROR(dwError);
            dwError = TDNFUtilsFormatSize(
                        pPkgInfo->dwInstallSizeBytes,
                        &pPkgInfo->pszFormattedSize);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgSummaryFromId(pSack, pkgId, &pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        pszSummary,
                        &pPkgInfo->pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgUrlFromId(pSack, pkgId, &pszUrl);
            dwError = TDNFSafeAllocateString(
                        pszUrl,
                        &pPkgInfo->pszURL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgLicenseFromId(pSack, pkgId, &pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFSafeAllocateString(
                        pszLicense,
                        &pPkgInfo->pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgDescriptionFromId(pSack, pkgId, &pszDescription);
            dwError = TDNFSafeAllocateString(
                        pszDescription,
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

uint32_t
TDNFPackageGetDowngrade(
    Id current,
    PTDNF pTdnf,
    Id* pkgId,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    int dwEvrCompare = 0;
    Id availableId = 0;
    Id downGradeId = 0;
    uint32_t dwCount = 0;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pTdnf)
    {
        return 0;
    }

    dwError = SolvCreatePackageList(&pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack,
                    pszPkgName,
                    pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        SolvGetPackageId(pAvailabePkgList, dwPkgIndex, &availableId);
        dwError = SolvCmpEvr(pTdnf->pSack, availableId, current, &dwEvrCompare);
        if(dwError == 0 && dwEvrCompare < 0)
        {
            if(downGradeId == 0)
            {
                downGradeId = availableId;
            }
            else
            {
                dwError = SolvCmpEvr(pTdnf->pSack, availableId, downGradeId, &dwEvrCompare);
                if(dwError == 0 && dwEvrCompare > 0)
                {
                    downGradeId = availableId;
                }
            }
        }
    }

    if(downGradeId == 0)
    {
        dwError = ERROR_TDNF_NO_DOWNGRADE_PATH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pkgId = downGradeId;
cleanup:
    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
    }

    return dwError;
error:
    *pkgId = 0;
    goto cleanup;
}

uint32_t
TDNFGetGlobPackages(
    PTDNF pTdnf,
    char* pszPkgGlob,
    Queue* pQueueGoal
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    PSolvPackageList pSolvPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack || IsNullOrEmptyString(pszPkgGlob) || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack,
                    pszPkgGlob,
                    pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pSolvPkgList->queuePackages.count > 0)
    {
        for(dwPkgIndex = 0; dwPkgIndex < pSolvPkgList->queuePackages.count; dwPkgIndex++)
        {
            queue_push(pQueueGoal, pSolvPkgList->queuePackages.elements[dwPkgIndex]);
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

Id SolvFindHighestAvailable(
    PTDNF pTdnf,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    int dwEvrCompare = 0;
    Id availableId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pAvailabePkgList = NULL;
    uint32_t dwCount = 0;

    if(!pTdnf)
    {
        return 0;
    }

    dwError = SolvCreatePackageList(&pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack,
                    pszPkgName,
                    pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    SolvGetPackageId(pAvailabePkgList, 0, &highestAvailable);
    if(highestAvailable != 0)
    {
        dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
        BAIL_ON_TDNF_ERROR(dwError);

        for(dwPkgIndex = 1; dwPkgIndex < dwCount; dwPkgIndex++)
        {
            SolvGetPackageId(pAvailabePkgList, dwPkgIndex, &availableId);
            dwError = SolvCmpEvr(pTdnf->pSack, availableId, highestAvailable, &dwEvrCompare);
            if(dwError == 0 && dwEvrCompare > 0)
            {
                highestAvailable = availableId;
            }
        }
    }
    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
    }
cleanup:
    return highestAvailable;
error:
    goto cleanup;
}

uint32_t
TDNFAddPackagesForErase(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id installedId = 0;
    int dwPkgIndex = 0;
    uint32_t dwCount = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pTdnf->pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &installedId);
        queue_push(pQueueGoal, installedId);
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
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwEvrCompare = 0;
    Id installedId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pTdnf->pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    SolvGetPackageId(pInstalledPkgList, 0, &installedId);

    highestAvailable = SolvFindHighestAvailable(pTdnf, pszPkgName);
    if(highestAvailable != 0)
    {
        if(installedId != 0)
        {
            dwError = SolvCmpEvr(pTdnf->pSack, highestAvailable, installedId, &dwEvrCompare);
            if(dwError == 0 && dwEvrCompare > 0)
            {
                queue_push(pQueueGoal, highestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_ALREADY_INSTALLED;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            queue_push(pQueueGoal, highestAvailable);
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
TDNFAddPackagesForUpgrade(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwEvrCompare = 0;
    Id installedId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    const char* pszName = NULL;

    if(!pTdnf || !pTdnf->pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                    pszPkgName,
                    pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    SolvGetPackageId(pInstalledPkgList, 0, &installedId);

    highestAvailable = SolvFindHighestAvailable(pTdnf, pszPkgName);
    if(highestAvailable != 0)
    {
        if(installedId == 0)
        {
            SolvEmptyPackageList(pInstalledPkgList);
            dwError = SolvGetPkgNameFromId(pTdnf->pSack, highestAvailable, &pszName);

            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                        pszName, pInstalledPkgList);
            if(dwError == ERROR_TDNF_NO_MATCH)
            {
                dwError = 0;
            }
            SolvGetPackageId(pInstalledPkgList, 0, &installedId);
        }

        if(installedId != 0)
        {
            dwError = SolvCmpEvr(pTdnf->pSack, highestAvailable, installedId, &dwEvrCompare);
            if(dwError == 0 && dwEvrCompare > 0)
            {
                queue_push(pQueueGoal, highestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            queue_push(pQueueGoal, highestAvailable);
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
TDNFAddPackagesForDowngrade(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id  pkg = 0;
    int dwEvrCompare = 0;

    PSolvPackageList pInstalledPkgList = NULL;
    Id installedId = 0;
    Id availableId = 0;
    const char* pszName = NULL;

    if(!pTdnf || !pQueueGoal || !pszPkgName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);
    
    dwError = SolvFindInstalledPkgByName(pTdnf->pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    SolvGetPackageId(pInstalledPkgList, 0, &installedId);
    if(installedId != 0)
    {
        dwError = TDNFPackageGetDowngrade(installedId, pTdnf, &pkg, pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push(pQueueGoal, pkg);
    }
    else
    {
        availableId = SolvFindHighestAvailable(pTdnf, pszPkgName);
        if(availableId != 0)
        {
            SolvEmptyPackageList(pInstalledPkgList);
            dwError = SolvGetPkgNameFromId(
                            pTdnf->pSack,
                            availableId,
                            &pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                            pszName,
                            pInstalledPkgList);
            BAIL_ON_TDNF_ERROR(dwError);

            SolvGetPackageId(pInstalledPkgList, 0, &installedId);
            if(installedId != 0)
            {
                dwError = SolvCmpEvr(pTdnf->pSack, availableId, installedId, &dwEvrCompare);
                if(dwError == 0 && dwEvrCompare < 0)
                {
                    queue_push(pQueueGoal, availableId);
                    pkg = availableId;
                }
            }

        }
    }

    if(pkg == 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
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
TDNFPopulatePkgInfos(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfos
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int dwPkgIndex = 0;
    Id  pkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    const char* pszName = NULL;
    const char* pszArch = NULL;
    const char* pszSummary = NULL;
    const char* pszLocation = NULL;
    const char* pszRepoName = NULL;
    const char* pszVersion = NULL;
    const char* pszRelease = NULL;

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
        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &pkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack, pkgId, &pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      pszName,
                      &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgVersionFromId(pSack, pkgId, &pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszVersion,
                    &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(pSack, pkgId, &pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                    pszRelease,
                    &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgArchFromId(pSack, pkgId, &pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      pszArch,
                      &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(pSack, pkgId, &pszRepoName);
        dwError = TDNFSafeAllocateString(
                      pszRepoName,
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgSummaryFromId(pSack, pkgId, &pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(
                      pszSummary,
                      &pPkgInfo->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgLocationFromId(pSack, pkgId, &pszLocation);
        dwError = TDNFSafeAllocateString(
                      pszLocation,
                      &pPkgInfo->pszLocation);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgInstallSizeFromId(pSack, 
                        pkgId, &pPkgInfo->dwInstallSizeBytes);

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
