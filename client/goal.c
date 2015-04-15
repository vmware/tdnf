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
    HyPackageList hPkgList,
    HySelector hSelector,
    TDNF_ALTERTYPE nResolveFor,
    PTDNF_SOLVED_PKG_INFO pInfo
    )
{
    uint32_t dwError = 0;
    HyGoal hGoal = NULL;
    HyPackage hPkg = NULL;
    PTDNF_PKG_INFO pPkgsToInstall = NULL;
    PTDNF_PKG_INFO pPkgsToUpgrade = NULL;
    PTDNF_PKG_INFO pPkgsToDowngrade = NULL;
    PTDNF_PKG_INFO pPkgsToRemove = NULL;
    PTDNF_PKG_INFO pPkgsUnNeeded = NULL;
    PTDNF_PKG_INFO pPkgsToReinstall = NULL;

    int nFlags = 0;

    if(!pTdnf || !hPkgList || !pInfo )
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nResolveFor == ALTER_UPGRADE && !hSelector)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hPkg = hy_packagelist_get(hPkgList, 0);
    if(!hPkg)
    {
        dwError = ERROR_TDNF_PACKAGELIST_EMPTY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hGoal = hy_goal_create(pTdnf->hSack);
    if(!hGoal)
    {
        dwError = ERROR_TDNF_GOAL_CREATE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    switch(nResolveFor)
    {
        case ALTER_DOWNGRADE:
            dwError = hy_goal_downgrade_to(hGoal, hPkg);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        case ALTER_ERASE:
            dwError = hy_goal_erase(hGoal, hPkg);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        case ALTER_REINSTALL:
        case ALTER_INSTALL:
            dwError = hy_goal_install(hGoal, hPkg);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        case ALTER_UPGRADE:
            dwError = hy_goal_upgrade_to_selector(hGoal, hSelector);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        case ALTER_UPGRADEALL:
            dwError = hy_goal_upgrade_all(hGoal);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        case ALTER_AUTOERASE:
            dwError = TDNFGoalSetUserInstalled(hGoal, hPkgList);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
        default:
            dwError = ERROR_TDNF_INVALID_RESOLVE_ARG;
            BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nBest)
    {
        nFlags = nFlags | HY_FORCE_BEST;
    }
    if(pTdnf->pArgs->nAllowErasing ||
       nResolveFor == ALTER_ERASE ||
       nResolveFor == ALTER_AUTOERASE)
    {
        nFlags = nFlags | HY_ALLOW_UNINSTALL;
    }

    dwError = hy_goal_run_flags(hGoal, nFlags);
    if(pTdnf->pArgs->nDebugSolver)
    {
        hy_goal_write_debugdata(hGoal, "debugdata");
    }
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    dwError = TDNFGoalGetResultsIgnoreNoData(
                  hy_goal_list_installs(hGoal),
                  &pPkgsToInstall);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGoalGetResultsIgnoreNoData(
                  hy_goal_list_upgrades(hGoal),
                  &pPkgsToUpgrade);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGoalGetResultsIgnoreNoData(
                  hy_goal_list_downgrades(hGoal),
                  &pPkgsToDowngrade);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGoalGetResultsIgnoreNoData(
                  hy_goal_list_erasures(hGoal),
                  &pPkgsToRemove);
    BAIL_ON_TDNF_ERROR(dwError);

    if(nResolveFor == ALTER_AUTOERASE)
    {
        dwError = TDNFGoalGetResultsIgnoreNoData(
                      hy_goal_list_unneeded(hGoal),
                      &pPkgsUnNeeded);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFGoalGetResultsIgnoreNoData(
                  hy_goal_list_reinstalls(hGoal),
                  &pPkgsToReinstall);
    BAIL_ON_TDNF_ERROR(dwError);

    pInfo->pPkgsToInstall = pPkgsToInstall;
    pInfo->pPkgsToUpgrade = pPkgsToUpgrade;
    pInfo->pPkgsToDowngrade = pPkgsToDowngrade;
    pInfo->pPkgsToRemove = pPkgsToRemove;
    pInfo->pPkgsUnNeeded = pPkgsUnNeeded;
    pInfo->pPkgsToReinstall = pPkgsToReinstall;
    pTdnf->hGoal = hGoal;

cleanup:
    return dwError;

error:
    if(hGoal)
    {
        TDNFGoalReportProblems(hGoal);
        hy_goal_free(hGoal);
    }
    if(pPkgsToInstall)
    {
        TDNFFreePackageInfo(pPkgsToInstall);
    }
    if(pPkgsToUpgrade)
    {
        TDNFFreePackageInfo(pPkgsToUpgrade);
    }
    if(pPkgsToDowngrade)
    {
        TDNFFreePackageInfo(pPkgsToDowngrade);
    }
    if(pPkgsToRemove)
    {
        TDNFFreePackageInfo(pPkgsToRemove);
    }
    if(pPkgsUnNeeded)
    {
        TDNFFreePackageInfo(pPkgsUnNeeded);
    }
    if(pPkgsToReinstall)
    {
        TDNFFreePackageInfo(pPkgsToReinstall);
    }
    goto cleanup;
}

uint32_t
TDNFGoalGetResultsIgnoreNoData(
    HyPackageList hPkgList,
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
    HyPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfoResults
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfoResults = NULL;

    if(!hPkgList || !ppPkgInfoResults)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPopulatePkgInfos(
                  hPkgList,
                  &pPkgInfoResults);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfoResults = pPkgInfoResults;
cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(ppPkgInfoResults)
    {
        *ppPkgInfoResults = NULL;
    }
    if(pPkgInfoResults)
    {
        TDNFFreePackageInfo(pPkgInfoResults);
    }
    goto cleanup;
}

uint32_t
TDNFGoalReportProblems(
    HyGoal hGoal
    )
{
    uint32_t dwError = 0;
    int i = 0;
    int nCount = 0;
    char* pszProblem = NULL;

    if(!hGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nCount = hy_goal_count_problems(hGoal);
    if(nCount > 0)
    {
        fprintf(stderr, "Found %d problem(s) while resolving\n", nCount);
        for(; i < nCount; ++i)
        {
            pszProblem = hy_goal_describe_problem(hGoal, i);
            fprintf(stderr, "%d. %s\n", i+1, pszProblem);

            hy_free(pszProblem);
            pszProblem = NULL;
        }
    }
cleanup:
    if(pszProblem)
    {
        hy_free(pszProblem);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGoalSetUserInstalled(
    HyGoal hGoal,
    HyPackageList hPkgList
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;

    if(!hGoal || !hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    FOR_PACKAGELIST(hPkg, hPkgList, i)
    {
        dwError = hy_goal_userinstalled(hGoal, hPkg);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
