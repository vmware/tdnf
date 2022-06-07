/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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

#define _GNU_SOURCE 1
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

    if(!pSack || !pQueueGoal || IsNullOrEmptyString(pszName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindInstalledPkgByName(
                  pSack,
                  pszName,
                  &pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError =  SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPkgNevrFromId(pSack, dwInstalledId, &pszNevr);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszNevr,
                  &pAvailabePkgList);
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
    uint32_t dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;
    char *pszSrcName = NULL;
    char *pszSrcArch = NULL;
    char *pszSrcEVR = NULL;

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

    for (dwPkgIndex = 0; (uint32_t)dwPkgIndex < dwCount; dwPkgIndex++)
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

            dwError = SolvGetPkgDownloadSizeFromId(
                        pSack,
                        dwPkgId,
                        &pPkgInfo->dwDownloadSizeBytes);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFUtilsFormatSize(
                          pPkgInfo->dwInstallSizeBytes,
                          &pPkgInfo->pszFormattedSize);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFUtilsFormatSize(
                          pPkgInfo->dwDownloadSizeBytes,
                          &pPkgInfo->pszFormattedDownloadSize);
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
        else if (nDetail == DETAIL_CHANGELOG)
        {
            dwError = SolvGetChangeLogFromId(
                          pSack,
                          dwPkgId,
                          &pPkgInfo->pChangeLogEntries);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (nDetail == DETAIL_SOURCEPKG)
        {
            dwError = SolvGetSourceFromId(
                          pSack,
                          dwPkgId,
                          &pszSrcName,
                          &pszSrcArch,
                          &pszSrcEVR);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFAllocateStringPrintf(
                          &pPkgInfo->pszSourcePkg,
                          "%s-%s.%s",
                          pszSrcName, pszSrcEVR, pszSrcArch);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (dwPkgIndex < dwCount - 1)
        {
            pPkgInfo->pNext = &pPkgInfos[dwPkgIndex+1];
        }
    }

    *pdwCount = dwCount;
    *ppPkgInfo = pPkgInfos;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszSrcName);
    TDNF_SAFE_FREE_MEMORY(pszSrcArch);
    TDNF_SAFE_FREE_MEMORY(pszSrcEVR);
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
TDNFPopulatePkgInfoForRepoSync(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfo
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    int dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

    if(!ppPkgInfo || !pSack || !pPkgList)
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

    for (dwPkgIndex = 0; (uint32_t)dwPkgIndex < dwCount; dwPkgIndex++)
    {
        pPkgInfo = &pPkgInfos[dwPkgIndex];
        if ((uint32_t)dwPkgIndex < dwCount-1)
        {
            pPkgInfo->pNext = &pPkgInfos[dwPkgIndex+1];
        }

        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetNevraFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->dwEpoch,
                      &pPkgInfo->pszName,
                      &pPkgInfo->pszVersion,
                      &pPkgInfo->pszRelease,
                      &pPkgInfo->pszArch,
                      &pPkgInfo->pszEVR);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgLocationFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->pszLocation);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppPkgInfo = pPkgInfos;

cleanup:
    return dwError;

error:
    if(ppPkgInfo)
    {
        *ppPkgInfo = NULL;
    }
    if(pPkgInfos)
    {
        TDNFFreePackageInfoArray(pPkgInfos, dwCount);
    }
    goto cleanup;
}

static
int _pkginfo_compare(
        const void *ptr1,
        const void *ptr2,
        void *data
    )
{
    const PTDNF_PKG_INFO* ppPkgInfo1 = (PTDNF_PKG_INFO*)ptr1;
    const PTDNF_PKG_INFO* ppPkgInfo2 = (PTDNF_PKG_INFO*)ptr2;
    Pool *pPool = (Pool *)data;
    int ret;

    /* sort by repo name first, then name, then version */
    ret = strcmp((*ppPkgInfo1)->pszRepoName, (*ppPkgInfo2)->pszRepoName);
    if (ret != 0)
    {
        return ret;
    }
    ret = strcmp((*ppPkgInfo1)->pszName, (*ppPkgInfo2)->pszName);
    if (ret != 0)
    {
        return ret;
    }

    /* we want newest version first, so reverse it by using the negated value */
    ret = - pool_evrcmp_str(pPool,
                          (*ppPkgInfo1)->pszEVR, (*ppPkgInfo2)->pszEVR,
                          EVRCMP_COMPARE);
    return ret;
}

uint32_t
TDNFPkgInfoFilterNewest(
    PSolvSack pSack,
    PTDNF_PKG_INFO pPkgInfos
)
{
    uint32_t dwError = 0;
    uint32_t dwCount, i;
    PTDNF_PKG_INFO* ppPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    dwCount = 0;
    for (pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        dwCount++;
    }

    dwError = TDNFAllocateMemory(
                  dwCount,
                  sizeof(PTDNF_PKG_INFO),
                  (void**)&ppPkgInfos);
    BAIL_ON_TDNF_ERROR(dwError);

    i = 0;
    for (pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        ppPkgInfos[i++] = pPkgInfo;
    }

    qsort_r(ppPkgInfos, dwCount,
            sizeof(PTDNF_PKG_INFO), _pkginfo_compare, (void *)pSack->pPool);

    /* Loop though pointer array, use the linked list to skip over
       older versions of the same packages. The linked list will only
       touch the newest (first) version of a package.
       The same package in different repos will be handled as two different
       packages. */
    pPkgInfo = ppPkgInfos[0];
    for (i = 1; i < dwCount; i++)
    {
        if ((strcmp(ppPkgInfos[i]->pszRepoName, pPkgInfo->pszRepoName) != 0) ||
            (strcmp(ppPkgInfos[i]->pszName, pPkgInfo->pszName) != 0))
        {
            pPkgInfo->pNext = ppPkgInfos[i];
            pPkgInfo = ppPkgInfos[i];
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(ppPkgInfos);
    return dwError;

error:
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
    Id dwInstalled,
    PSolvSack pSack,
    PSolvPackageList pAvailabePkgList,
    Id* pdwDowngradePkgId
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
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; (uint32_t)dwPkgIndex < dwCount; dwPkgIndex++)
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
                dwError = SolvCmpEvr(
                              pSack,
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
    PSolvPackageList pSolvPkgList = NULL;

    if(!pSack || IsNullOrEmptyString(pszPkgGlob) || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszPkgGlob,
                  &pSolvPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pSolvPkgList->queuePackages.count > 0)
    {
        for(int dwPkgIndex = 0;
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

    if(!pSack || !pQueueGoal || IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindInstalledPkgByName(
                  pSack,
                  pszPkgName,
                  &pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; (uint32_t)dwPkgIndex < dwCount; dwPkgIndex++)
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
TDNFVerifyInstallPackage(
    PSolvSack pSack,
    Id dwPkg,
    uint32_t* pdwInstallPackage
    )
{

    uint32_t dwError = 0;
    char* pszName = NULL;
    Id  dwInstalledId = 0;
    int dwEvrCompare = 0;
    uint32_t dwInstallPackage = 0;

    if(!pSack || !pdwInstallPackage)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgNameFromId(pSack, dwPkg, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindHighestInstalled(
                  pSack,
                  pszName,
                  &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCmpEvr(pSack, dwPkg, dwInstalledId, &dwEvrCompare);
    BAIL_ON_TDNF_ERROR(dwError);

    //allow updates and downgrades with install
    //install could specify version
    if(dwEvrCompare)
    {
        dwInstallPackage = 1;
    }

    *pdwInstallPackage = dwInstallPackage;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    return dwError;

error:
    if((dwError == ERROR_TDNF_NO_MATCH || dwError == ERROR_TDNF_NO_DATA) &&
       pdwInstallPackage)
    {
        *pdwInstallPackage = 1;
        dwError = 0;
    }
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
    Id dwHighestAvailable = 0;
    uint32_t  dwInstallPackage = 0;

    if(!pSack || !pQueueGoal || IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindHighestAvailable(
                  pSack,
                  pszPkgName,
                  &dwHighestAvailable);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFVerifyInstallPackage(
                  pSack,
                  dwHighestAvailable,
                  &dwInstallPackage);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwInstallPackage == 1)
    {
        queue_push(pQueueGoal, dwHighestAvailable);
    }
    else
    {
        dwError = ERROR_TDNF_ALREADY_INSTALLED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}


uint32_t
TDNFVerifyUpgradePackage(
    PSolvSack pSack,
    Id dwPkg,
    uint32_t* pdwUpgradePackage
    )
{

    uint32_t dwError = 0;
    char* pszName = NULL;
    Id  dwInstalledId = 0;
    int dwEvrCompare = 0;
    uint32_t dwUpgradePackage = 0;

    if(!pSack || !pdwUpgradePackage)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgNameFromId(pSack, dwPkg, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindHighestInstalled(
                  pSack,
                  pszName,
                  &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCmpEvr(pSack, dwPkg, dwInstalledId, &dwEvrCompare);
    if(dwError == 0 && dwEvrCompare > 0)
    {
        dwUpgradePackage = 1;
    }
    else
    {
        dwUpgradePackage = 0;
    }
    *pdwUpgradePackage = dwUpgradePackage;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    return dwError;

error:
    if(pdwUpgradePackage)
    {
        *pdwUpgradePackage = 0;
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

    if(!pSack || !pQueueGoal || IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindHighestAvailable(
                  pSack,
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

    if(!pSack || !pQueueGoal || IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszPkgName,
                  &pAvailabePkgList);
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
TDNFHasProtectedPkgsInList(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    uint32_t dwError = 0;

    if(!pPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        if(pPkgInfo->pszName && !strcmp(pPkgInfo->pszName, TDNF_NAME))
        {
            dwError = ERROR_TDNF_SELF_ERASE;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
error:
    return dwError;
}

uint32_t
TDNFCheckProtectedPkgs(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;

    if(!pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = TDNFHasProtectedPkgsInList(pSolvedPkgInfo->pPkgsToRemove);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = TDNFHasProtectedPkgsInList(pSolvedPkgInfo->pPkgsObsoleted);
        BAIL_ON_TDNF_ERROR(dwError);
    }

error:
    return dwError;
}

uint32_t
TDNFGetAvailableCacheBytes(
    PTDNF_CONF pConf,
    uint64_t* pqwAvailCacheDirBytes
    )
{
    uint32_t dwError = 0;
    struct statfs tmpStatfsBuffer = {0};

    if(!pConf || !pConf->pszCacheDir || !pqwAvailCacheDirBytes)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (statfs(pConf->pszCacheDir, &tmpStatfsBuffer) != 0)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    *pqwAvailCacheDirBytes = tmpStatfsBuffer.f_bsize * tmpStatfsBuffer.f_bavail;

cleanup:
    return dwError;

error:
    if(pqwAvailCacheDirBytes)
    {
        *pqwAvailCacheDirBytes = 0;
    }
    goto cleanup;
}

uint32_t
TDNFCheckDownloadCacheBytes(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    uint64_t qwAvailCacheBytes
    )
{
    uint32_t dwError = 0;
    uint64_t qwTotalDownloadSizeBytes = 0;
    uint8_t byPkgIndex = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    PTDNF_PKG_INFO ppPkgsNeedDownload[4] = {
        pSolvedPkgInfo->pPkgsToInstall,
        pSolvedPkgInfo->pPkgsToDowngrade,
        pSolvedPkgInfo->pPkgsToUpgrade,
        pSolvedPkgInfo->pPkgsToReinstall
    };

    for (byPkgIndex = 0; byPkgIndex < ARRAY_SIZE(ppPkgsNeedDownload); byPkgIndex++)
    {
        pPkgInfo = ppPkgsNeedDownload[byPkgIndex];
        while(pPkgInfo) {
            qwTotalDownloadSizeBytes += pPkgInfo->dwDownloadSizeBytes;
            if (qwTotalDownloadSizeBytes > qwAvailCacheBytes)
            {
                dwError = ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            pPkgInfo = pPkgInfo->pNext;
        }
    }

error:
    return dwError;
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

    for (dwPkgIndex = 0; (uint32_t)dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetNevraFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->dwEpoch,
                      &pPkgInfo->pszName,
                      &pPkgInfo->pszVersion,
                      &pPkgInfo->pszRelease,
                      &pPkgInfo->pszArch,
                      &pPkgInfo->pszEVR);
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

        dwError = SolvGetPkgDownloadSizeFromId(
                      pSack,
                      dwPkgId,
                      &pPkgInfo->dwDownloadSizeBytes);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFUtilsFormatSize(
                      pPkgInfo->dwInstallSizeBytes,
                      &pPkgInfo->pszFormattedSize);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFUtilsFormatSize(
                      pPkgInfo->dwDownloadSizeBytes,
                      &pPkgInfo->pszFormattedDownloadSize);
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
TDNFPopulatePkgInfoArrayDependencies(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    REPOQUERY_DEP_KEY depKey,
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    uint32_t dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

    if(!pPkgInfos || !pSack || !pPkgList)
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

    for (dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        pPkgInfo = &pPkgInfos[dwPkgIndex];

        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetDependenciesFromId(pSack, dwPkgId, depKey, &pPkgInfo->ppszDependencies);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPopulatePkgInfoArrayFileList(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    uint32_t dwPkgIndex = 0;
    Id dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

    if(!pPkgInfos || !pSack || !pPkgList)
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

    for (dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        pPkgInfo = &pPkgInfos[dwPkgIndex];

        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetFileListFromId(pSack, dwPkgId, &pPkgInfo->ppszFileList);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

