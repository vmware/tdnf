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
    Queue* qGoal
    )
{
    uint32_t dwError = 0;
    int pkgIndex = 0;
    int evrCompare = 0;
    int pkgNotFound = 0;
    Id installedId = 0;
    Id availableId = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pSack || !qGoal)
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
    pAvailabePkgList = SolvCreatePackageList();
    if(!pAvailabePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindInstalledPkgByName(pSack, pszName, pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);
    dwError = SolvFindAvailablePkgByName(pSack, pszName, pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError =  SolvGetPackageId(pInstalledPkgList, 0, &installedId);
    BAIL_ON_TDNF_ERROR(dwError);
    for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pAvailabePkgList); pkgIndex++)
    {
        dwError = SolvGetPackageId(pAvailabePkgList, 0, &availableId);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = SolvCmpEvr(pSack, installedId, availableId, &evrCompare);
        if(dwError == 0 && evrCompare == 0)
        {
            queue_push(qGoal, availableId);
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

uint32_t
TDNFPackageGetDowngrade(
    Id current,
    PTDNF pTdnf,
    Id* pkgId,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int pkgIndex = 0;
    int evrCompare = 0;
    Id availableId = 0;
    Id downGradeId = 0;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pTdnf)
    {
        return 0;
    }

    pAvailabePkgList = SolvCreatePackageList();
    if(!pAvailabePkgList)
    {
        return 0;
    }

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack, pszPkgName, pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);


    for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pAvailabePkgList); pkgIndex++)
    {
        SolvGetPackageId(pAvailabePkgList, pkgIndex, &availableId);
        dwError = SolvCmpEvr(pTdnf->pSack, availableId, current, &evrCompare);
        if(dwError == 0 && evrCompare < 0)
        {
            if(downGradeId == 0)
            {
                downGradeId = availableId;
            }
            else
            {
                dwError = SolvCmpEvr(pTdnf->pSack, availableId, downGradeId, &evrCompare);
                if(dwError == 0 && evrCompare > 0)
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
    Queue* qGoal
    )
{
    uint32_t dwError = 0;
    int pkgIndex = 0;
    PSolvPackageList pSolvPkgList = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszPkgGlob) || !qGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSolvPkgList = SolvCreatePackageList();
    if(!pSolvPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack, pszPkgGlob, pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pSolvPkgList->packages.count > 0)
    {
        for(pkgIndex = 0; pkgIndex < pSolvPkgList->packages.count; pkgIndex++)
        {
            queue_push(qGoal, pSolvPkgList->packages.elements[pkgIndex]);
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
    int pkgIndex = 0;
    int evrCompare = 0;
    Id availableId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pAvailabePkgList = NULL;

    if(!pTdnf)
    {
        return 0;
    }

    pAvailabePkgList = SolvCreatePackageList();
    if(!pAvailabePkgList)
    {
        return 0;
    }

    dwError = SolvFindAvailablePkgByName(pTdnf->pSack, pszPkgName, pAvailabePkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    SolvGetPackageId(pAvailabePkgList, 0, &highestAvailable);
    if(highestAvailable != 0)
    {
        for(pkgIndex = 1; pkgIndex < SolvGetPackageListSize(pAvailabePkgList); pkgIndex++)
        {
            SolvGetPackageId(pAvailabePkgList, pkgIndex, &availableId);
            dwError = SolvCmpEvr(pTdnf->pSack, availableId, highestAvailable, &evrCompare);
            if(dwError == 0 && evrCompare > 0)
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
    Queue* qGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id installedId = 0;
    int pkgIndex = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !qGoal)
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
    dwError = SolvFindInstalledPkgByName(pTdnf->pSack, pszPkgName, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pInstalledPkgList); pkgIndex++)
    {
        SolvGetPackageId(pInstalledPkgList, pkgIndex, &installedId);
        queue_push(qGoal, installedId);
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
    Queue* qGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int evrCompare = 0;
    Id installedId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !qGoal)
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
            dwError = SolvCmpEvr(pTdnf->pSack, highestAvailable, installedId, &evrCompare);
            if(dwError == 0 && evrCompare > 0)
            {
                queue_push(qGoal, highestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_ALREADY_INSTALLED;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            queue_push(qGoal, highestAvailable);
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
    Queue* qGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int evrCompare = 0;
    Id installedId = 0;
    Id highestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !qGoal)
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
    dwError = SolvFindInstalledPkgByName(pTdnf->pSack, pszPkgName, pInstalledPkgList);
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
            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                        SolvGetPkgNameFromId(pTdnf->pSack, highestAvailable), pInstalledPkgList);
            if(dwError == ERROR_TDNF_NO_MATCH)
            {
                dwError = 0;
            }
            SolvGetPackageId(pInstalledPkgList, 0, &installedId);
        }

        if(installedId != 0)
        {
            dwError = SolvCmpEvr(pTdnf->pSack, highestAvailable, installedId, &evrCompare);
            if(dwError == 0 && evrCompare > 0)
            {
                queue_push(qGoal, highestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            queue_push(qGoal, highestAvailable);
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
    Queue* qGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id  pkg = 0;
    int evrCompare = 0;

    PSolvPackageList pInstalledPkgList = NULL;
    Id installedId = 0;
    Id availableId = 0;

    if(!pTdnf || !qGoal || !pszPkgName)
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
        queue_push(qGoal, pkg);
    }
    else
    {
        availableId = SolvFindHighestAvailable(pTdnf, pszPkgName);
        if(availableId != 0)
        {
            SolvEmptyPackageList(pInstalledPkgList);
            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                        SolvGetPkgNameFromId(pTdnf->pSack, availableId), pInstalledPkgList);
            if(dwError == ERROR_TDNF_NO_MATCH)
            {
                dwError = 0;
            }
            SolvGetPackageId(pInstalledPkgList, 0, &installedId);
            if(installedId != 0)
            {
                dwError = SolvCmpEvr(pTdnf->pSack, availableId, installedId, &evrCompare);
                if(dwError == 0 && evrCompare < 0)
                {
                    queue_push(qGoal, availableId);
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
