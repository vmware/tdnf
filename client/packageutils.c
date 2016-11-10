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
    int dwPkgNotFound = 0;
    Id  dwInstalledId = 0;
    Id  dwAvailableId = 0;
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

    dwError =  SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pAvailabePkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pAvailabePkgList, 0, &dwAvailableId);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = SolvCmpEvr(pSack, dwInstalledId, dwAvailableId, &dwEvrCompare);
        if(dwError == 0 && dwEvrCompare == 0)
        {
            queue_push(pQueueGoal, dwAvailableId);
            dwPkgNotFound = 1;
            break;
        }
    }

    if(dwPkgNotFound == 0)
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
    Id  dwPkgId = 0;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo  = NULL;

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
        PTDNF_PKG_INFO pPkgInfo = &pPkgInfos[dwPkgIndex];

        dwError = SolvGetPackageId(pPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack, dwPkgId, &pPkgInfo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgArchFromId(pSack, dwPkgId, &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgVersionFromId(pSack, dwPkgId, &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(pSack, dwPkgId, &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(pSack,
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

            dwError = SolvGetPkgSummaryFromId(pSack,
                                              dwPkgId,
                                              &pPkgInfo->pszSummary);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgUrlFromId(pSack, dwPkgId, &pPkgInfo->pszURL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgLicenseFromId(pSack,
                                              dwPkgId,
                                              &pPkgInfo->pszLicense);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPkgDescriptionFromId(pSack,
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
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
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
    Id    dwCurrent,
    PTDNF pTdnf,
    Id*   pDwPkgId,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwPkgIndex = 0;
    int dwEvrCompare = 0;
    Id  dwAvailableId = 0;
    Id  dwDownGradeId = 0;
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
        dwError = SolvGetPackageId(pAvailabePkgList, dwPkgIndex, &dwAvailableId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvCmpEvr(pTdnf->pSack, dwAvailableId, dwCurrent, &dwEvrCompare);
        BAIL_ON_TDNF_ERROR(dwError);

        if(dwEvrCompare < 0)
        {
            if(dwDownGradeId == 0)
            {
                dwDownGradeId = dwAvailableId;
            }
            else
            {
                dwError = SolvCmpEvr(pTdnf->pSack,
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

    *pDwPkgId = dwDownGradeId;
cleanup:
    if(pAvailabePkgList)
    {
        SolvFreePackageList(pAvailabePkgList);
    }

    return dwError;
error:
    *pDwPkgId = 0;
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

uint32_t
TDNFAddPackagesForErase(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    Id  dwInstalledId = 0;
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
        SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwInstalledId);
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
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwEvrCompare = 0;
    Id dwInstalledId = 0;
    Id dwHighestAvailable = 0;
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
    SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);

    SolvFindHighestAvailable(pTdnf->pSack, pszPkgName, &dwHighestAvailable);
    if(dwHighestAvailable != 0)
    {
        if(dwInstalledId != 0)
        {
            dwError = SolvCmpEvr(pTdnf->pSack,
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
TDNFAddPackagesForUpgrade(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0;
    int dwEvrCompare = 0;
    Id dwInstalledId = 0;
    Id dwHighestAvailable = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    char* pszName = NULL;

    if(!pTdnf || !pTdnf->pSack || !pQueueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindHighestAvailable(pTdnf->pSack,
                    pszPkgName,
                    &dwHighestAvailable);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                    pszPkgName,
                    pInstalledPkgList);

    if(dwError == 0)
    {
        dwError = SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
    }

    if(dwHighestAvailable != 0)
    {
        if(dwInstalledId == 0)
        {
            SolvEmptyPackageList(pInstalledPkgList);

            dwError = SolvGetPkgNameFromId(pTdnf->pSack, dwHighestAvailable, &pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                        pszName, pInstalledPkgList);
            if(dwError == ERROR_TDNF_NO_MATCH)
            {
                dwError = 0;
            }
            BAIL_ON_TDNF_ERROR(dwError);

            SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
        }

        if(dwInstalledId != 0)
        {
            dwError = SolvCmpEvr(pTdnf->pSack, dwHighestAvailable, dwInstalledId, &dwEvrCompare);
            if(dwError == 0 && dwEvrCompare > 0)
            {
                queue_push(pQueueGoal, dwHighestAvailable);
            }
            else
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;;
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
    TDNF_SAFE_FREE_MEMORY(pszName);
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
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
    Id  dwPkg = 0;
    int dwEvrCompare = 0;

    PSolvPackageList pInstalledPkgList = NULL;
    Id dwInstalledId = 0;
    Id dwAvailableId = 0;
    char* pszName = NULL;

    if(!pTdnf || !pQueueGoal || !pszPkgName)
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
    BAIL_ON_TDNF_ERROR(dwError);

    SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
    if(dwInstalledId != 0)
    {
        dwError = TDNFPackageGetDowngrade(dwInstalledId,
                            pTdnf,
                            &dwPkg,
                            pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push(pQueueGoal, dwPkg);
    }
    else
    {
        SolvFindHighestAvailable(pTdnf->pSack, pszPkgName, &dwAvailableId);
        if(dwAvailableId != 0)
        {
            SolvEmptyPackageList(pInstalledPkgList);
            dwError = SolvGetPkgNameFromId(
                            pTdnf->pSack,
                            dwAvailableId,
                            &pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                            pszName,
                            pInstalledPkgList);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
            BAIL_ON_TDNF_ERROR(dwError);

            if(dwInstalledId != 0)
            {
                dwError = SolvCmpEvr(pTdnf->pSack, dwAvailableId, dwInstalledId, &dwEvrCompare);
                if(dwError == 0 && dwEvrCompare < 0)
                {
                    queue_push(pQueueGoal, dwAvailableId);
                    dwPkg = dwAvailableId;
                }
            }

        }
    }

    if(dwPkg == 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
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
    Id  dwPkgId = 0;
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

        dwError = SolvGetPkgVersionFromId(pSack, dwPkgId, &pPkgInfo->pszVersion);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgReleaseFromId(pSack, dwPkgId, &pPkgInfo->pszRelease);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgArchFromId(pSack, dwPkgId, &pPkgInfo->pszArch);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgRepoNameFromId(pSack, dwPkgId, &pPkgInfo->pszRepoName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgSummaryFromId(pSack, dwPkgId, &pPkgInfo->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgLocationFromId(pSack, dwPkgId, &pPkgInfo->pszLocation);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgInstallSizeFromId(pSack,
                        dwPkgId, &pPkgInfo->dwInstallSizeBytes);

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
