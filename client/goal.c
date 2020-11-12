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
TDNFGetPackagesWithSpecifiedType(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    Id dwType)
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PSolvPackageList pPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack|| !pTrans || !pPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetTransResultsWithType(
                  pTrans,
                  dwType,
                  &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pPkgList, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwCount > 0)
    {
        dwError = TDNFPopulatePkgInfos(
                      pTdnf->pSack,
                      pPkgList,
                      pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetInstallPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(
               pTrans,
               pTdnf,
               pPkgInfo,
               SOLVER_TRANSACTION_INSTALL);
}

uint32_t
TDNFGetReinstallPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(
               pTrans,
               pTdnf,
               pPkgInfo,
               SOLVER_TRANSACTION_REINSTALL);
}

uint32_t
TDNFGetUpgradePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(
               pTrans,
               pTdnf,
               pPkgInfo,
               SOLVER_TRANSACTION_UPGRADE);
}

uint32_t
TDNFGetErasePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(
               pTrans,
               pTdnf,
               pPkgInfo,
               SOLVER_TRANSACTION_ERASE);
}

uint32_t
TDNFGetObsoletedPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(
               pTrans,
               pTdnf,
               pPkgInfo,
               SOLVER_TRANSACTION_OBSOLETED);
}

uint32_t
TDNFGetUnneededPackages(
    Solver* pSolv,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    uint32_t dwError = 0;
    PSolvPackageList pPkgList = NULL;
    Queue queueResult = {0};

    if(!pTdnf || !pTdnf->pSack|| !pSolv || !pPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&queueResult);
    solver_get_unneeded(pSolv, &queueResult, 0);

    if(queueResult.count > 0)
    {
        dwError = SolvQueueToPackageList(&queueResult, &pPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFPopulatePkgInfos(
                      pTdnf->pSack,
                      pPkgList,
                      pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    queue_free(&queueResult);
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetDownGradePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    PTDNF_PKG_INFO* pRemovePkgInfo)
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    Id dwInstalledId = 0;
    PSolvPackageList pRemovePkgList = NULL;
    PTDNF_PKG_INFO pInfo = NULL;
    Queue queuePkgToRemove = {0};

    if(!pTdnf || !pTdnf->pSack|| !pTrans || !pPkgInfo || !pRemovePkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&queuePkgToRemove);
    dwError = TDNFGetPackagesWithSpecifiedType(
                  pTrans,
                  pTdnf,
                  pPkgInfo,
                  SOLVER_TRANSACTION_DOWNGRADE);
    BAIL_ON_TDNF_ERROR(dwError);
    pInfo = *pPkgInfo;
    if(!pInfo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(pInfo)
    {
        dwError = SolvFindInstalledPkgByName(
                      pTdnf->pSack,
                      pInfo->pszName,
                      &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push(&queuePkgToRemove, dwInstalledId);
        pInfo = pInfo->pNext;
        SolvFreePackageList(pInstalledPkgList);
        pInstalledPkgList = NULL;
    }

    if(queuePkgToRemove.count > 0)
    {
        dwError = SolvQueueToPackageList(&queuePkgToRemove, &pRemovePkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFPopulatePkgInfos(
                      pTdnf->pSack,
                      pRemovePkgList,
                      pRemovePkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    queue_free(&queuePkgToRemove);
    if(pRemovePkgList)
    {
        SolvFreePackageList(pRemovePkgList);
    }
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
SolvAddDebugInfo(
    Solver *pSolv,
    const char *pszDir
    )
{
    uint32_t dwError = 0;
    uint32_t dwResultFlags = TESTCASE_RESULT_TRANSACTION |
                             TESTCASE_RESULT_PROBLEMS;
    if(!pSolv || IsNullOrEmptyString(pszDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //returns 1 for success.
    dwError = testcase_write(pSolv, pszDir, dwResultFlags, NULL, NULL);
    if(dwError == 0)
    {
        fprintf(stderr, "Could not write debugdata to folder %s\n", pszDir);
    }
    //need not fail if debugdata write fails.
    dwError = 0;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGoal(
    PTDNF pTdnf,
    Queue* pQueuePkgList,
    PTDNF_SOLVED_PKG_INFO* ppInfo,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;

    PTDNF_SOLVED_PKG_INFO pInfoTemp = NULL;
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem = SKIPPROBLEM_NONE;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    Queue queueJobs = {0};

    int nFlags = 0;
    int i = 0;
    Id  dwId = 0;
    int nProblems = 0;
    char** ppszExcludes = NULL;
    uint32_t dwExcludeCount = 0;

    if(!pTdnf || !ppInfo || !pQueuePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPkgsToExclude(pTdnf, &dwExcludeCount, &ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_init(&queueJobs);
    if (nAlterType == ALTER_UPGRADEALL)
    {
        dwError = SolvAddUpgradeAllJob(&queueJobs);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(nAlterType == ALTER_DISTRO_SYNC)
    {
        dwError = SolvAddDistUpgradeJob(&queueJobs);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        if (pQueuePkgList->count == 0)
        {
            dwError = ERROR_TDNF_ALREADY_INSTALLED;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        for (i = 0; i < pQueuePkgList->count; i++)
        {
            dwId = pQueuePkgList->elements[i];
            TDNFAddGoal(pTdnf, nAlterType, &queueJobs, dwId,
                        dwExcludeCount, ppszExcludes);
        }
    }

    if(pTdnf->pArgs->nBest)
    {
        nFlags = nFlags | SOLVER_FORCEBEST;
    }
    dwError = SolvAddFlagsToJobs(&queueJobs, nFlags);
    BAIL_ON_TDNF_ERROR(dwError);

    if (dwExcludeCount != 0 && ppszExcludes)
    {
        if (!pTdnf->pSack || !pTdnf->pSack->pPool)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        dwError = SolvAddExcludes(pTdnf->pSack->pPool, ppszExcludes);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSolv = solver_create(pTdnf->pSack->pPool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nAllowErasing ||
       nAlterType == ALTER_ERASE ||
       nAlterType == ALTER_AUTOERASE)
    {
        solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_UNINSTALL, 1);
    }
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_VENDORCHANGE, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_KEEP_ORPHANS, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_YUM_OBSOLETES, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_INSTALL_ALSO_UPDATES, 1);

    nProblems = solver_solve(pSolv, &queueJobs);
    if (nProblems > 0)
    {
        dwError = TDNFGetSkipProblemOption(pTdnf, &dwSkipProblem);
        BAIL_ON_TDNF_ERROR(dwError);

        if (nAlterType == ALTER_UPGRADE && dwExcludeCount != 0 && ppszExcludes)
        {
            /* if we had packages to exclude, then we'd have diabled ones too */
            dwSkipProblem |= SKIPPROBLEM_DISABLED;
        }

        dwError = SolvReportProblems(pTdnf->pSack, pSolv, dwSkipProblem);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pTrans = solver_create_transaction(pSolv);
    if(!pTrans)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nDebugSolver)
    {
        dwError = SolvAddDebugInfo(pSolv, "debugdata");
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGoalGetAllResultsIgnoreNoData(
                  nAlterType,
                  pTrans,
                  pSolv,
                  &pInfoTemp,
                  pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppInfo = pInfoTemp;

cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    queue_free(&queueJobs);
    if(pTrans)
    {
        transaction_free(pTrans);
    }
    if(pSolv)
    {
        solver_free(pSolv);
    }
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pInfoTemp);
    if(ppInfo)
    {
        *ppInfo = NULL;
    }
    goto cleanup;
}


uint32_t
TDNFAddGoal(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    Queue* pQueueJobs,
    Id dwId,
    uint32_t dwCount,
    char** ppszExcludes
    )
{
    uint32_t dwError = 0;
    char* pszPkg = NULL;
    char** ppszPackagesTemp = NULL;
    char* pszName = NULL;

    if(!pQueueJobs || dwId == 0 || !pTdnf->pSack || !pTdnf->pSack->pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (dwCount != 0 && ppszExcludes)
    {
        dwError = SolvGetPkgNameFromId(
                      pTdnf->pSack,
                      dwId,
                      &pszName);
        BAIL_ON_TDNF_ERROR(dwError);
        ppszPackagesTemp = ppszExcludes;

        while(ppszPackagesTemp && *ppszPackagesTemp)
        {
            if (SolvIsGlob(*ppszPackagesTemp))
            {
                if (!fnmatch(*ppszPackagesTemp, pszName, 0))
                {
                    goto cleanup;
                }
            }
            else if (!strcmp(pszName, *ppszPackagesTemp))
            {
                goto cleanup;
            }
            ++ppszPackagesTemp;
        }
    }

    switch(nAlterType)
    {
        case ALTER_DOWNGRADEALL:
        case ALTER_DOWNGRADE:
            dwError = SolvAddPkgDowngradeJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        case ALTER_ERASE:
            dwError = SolvAddPkgEraseJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        case ALTER_REINSTALL:
        case ALTER_INSTALL:
        case ALTER_UPGRADE:
            dwError = SolvAddPkgInstallJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        case ALTER_AUTOERASE:
            dwError = SolvAddPkgUserInstalledJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        default:
            dwError = ERROR_TDNF_INVALID_RESOLVE_ARG;
            BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPkg);
    TDNF_SAFE_FREE_MEMORY(pszName);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGoalGetAllResultsIgnoreNoData(
    int nResolveFor,
    Transaction* pTrans,
    Solver* pSolv,
    PTDNF_SOLVED_PKG_INFO* ppInfo,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_SOLVED_PKG_INFO pInfo = NULL;

    if(!pTrans || !pSolv || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_SOLVED_PKG_INFO),
                  (void**)&pInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetInstallPackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsToInstall);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetUpgradePackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsToUpgrade);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetDownGradePackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsToDowngrade,
                  &pInfo->pPkgsRemovedByDowngrade);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetErasePackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsToRemove);
    BAIL_ON_TDNF_ERROR(dwError);

    if(nResolveFor == ALTER_AUTOERASE)
    {
        dwError = TDNFGetUnneededPackages(
                      pSolv,
                      pTdnf,
                      &pInfo->pPkgsUnNeeded);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFGetReinstallPackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsToReinstall);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetObsoletedPackages(
                  pTrans,
                  pTdnf,
                  &pInfo->pPkgsObsoleted);
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
        TDNFFreeSolvedPackageInfo(pInfo);
    }
    goto cleanup;
}
