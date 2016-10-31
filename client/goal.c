/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : goal.c
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
TDNFGoal(
    PTDNF pTdnf,
    PSolvPackageList hPkgList,
    PTDNF_SOLVED_PKG_INFO pInfo
    )
{
    return 1;
}

uint32_t
TDNFAddGoal(
    PTDNF pTdnf,
    int nAlterType,
    PSolvGoal hGoal,
    PSolvPackage hPkg
    )
{
    return 1;
}

uint32_t
TDNFGoalGetAllResultsIgnoreNoData(
    int nResolveFor,
    PSolvGoal hGoal,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    )
{
    return 1;
}

uint32_t
TDNFGetAllResultsIgnoreNoData(
    int nResolveFor,
    PSolvQuery pQuery,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_SOLVED_PKG_INFO pInfo = NULL;
    PTDNF_PKG_INFO* pPkgInfo = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFAllocateMemory(
                1,
                sizeof(TDNF_SOLVED_PKG_INFO),
                (void**)&pInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pPkgList = SolvCreatePackageList();
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    switch(nResolveFor)
    {
        case ALTER_ERASE:
            dwError = SolvGetEraseResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToRemove;
            break;
        case ALTER_INSTALL:
            dwError = SolvGetInstallResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToInstall;
            break;
        case ALTER_UPGRADE:
        case ALTER_UPGRADEALL:
            dwError = SolvGetUpgradeResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToUpgrade;
            break;
        case ALTER_DOWNGRADE:
        case ALTER_DOWNGRADEALL:
            dwError = SolvGetDowngradeResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToDowngrade;
            break;
        case ALTER_DISTRO_SYNC:
            dwError = SolvGetUpgradeResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToUpgrade;
            break;
        case ALTER_REINSTALL:
            dwError = SolvGetReinstallResult(pQuery, pPkgList);
            pPkgInfo = &pInfo->pPkgsToInstall;
            break;
        default:
            break;
    }

    if(pPkgInfo == NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    
    dwError = TDNFPopulatePkgInfos(
                  pQuery->pSack,
                  pPkgList,
                  pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppInfo = pInfo;
cleanup:
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    return dwError;

error:
    if(ppInfo)
    {
        *ppInfo = NULL;
    }
    if(pInfo)
    {
        TDNFFreeSolvedPackageInfo(pInfo);
    }
    goto cleanup;
}

uint32_t
TDNFGoalGetResultsIgnoreNoData(
    PSolvPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfoResults
    )
{
    uint32_t dwError = 0;
    dwError = TDNFGoalGetResults(hPkgList, ppPkgInfoResults);
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    return dwError;
}

uint32_t
TDNFGoalGetResults(
    PSolvPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfoResults
    )
{
    return 1;
}

uint32_t
TDNFGoalReportProblems(
    PSolvGoal hGoal
    )
{
    return 1;
}

uint32_t
TDNFGoalSetUserInstalled(
    PSolvGoal hGoal,
    PSolvPackageList hPkgList
    )
{
    return 1;
}
