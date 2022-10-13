/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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
        pr_err("Could not write debugdata to folder %s\n", pszDir);
    }
    //need not fail if debugdata write fails.
    dwError = 0;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
uint32_t
TDNFAddUserInstalledToJobs(
    PTDNF pTdnf,
    Queue* pQueueJobs
    )
{
    uint32_t dwError = 0;
    struct history_ctx *pHistoryCtx = NULL;

    if(!pTdnf || !pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetHistoryCtx(pTdnf, &pHistoryCtx, 1);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvAddUserInstalledToJobs(pQueueJobs,
                                         pTdnf->pSack->pPool,
                                         pHistoryCtx);
    BAIL_ON_TDNF_ERROR(dwError);
cleanup:
    if (pHistoryCtx)
    {
        destroy_history_ctx(pHistoryCtx);
    }
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
TDNFSolv(
    PTDNF pTdnf,
    Queue *pQueueJobs,
    char** ppszExcludes,
    uint32_t dwExcludeCount,
    int nAllowErasing,
    int nAutoErase,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_SOLVED_PKG_INFO pInfo = NULL;
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem = SKIPPROBLEM_NONE;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    int nFlags = 0;
    int nProblems = 0;

    if(!pTdnf || !ppInfo || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nBest)
    {
        nFlags = nFlags | SOLVER_FORCEBEST;
    }
    if (nAutoErase)
    {
        nFlags = nFlags | SOLVER_CLEANDEPS;
    }

    dwError = SolvAddFlagsToJobs(pQueueJobs, nFlags);
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

    dwError = TDNFSolvAddPkgLocks(pTdnf, pQueueJobs, pTdnf->pSack->pPool);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSolvAddMinVersions(pTdnf, pQueueJobs, pTdnf->pSack->pPool);
    BAIL_ON_TDNF_ERROR(dwError);

    pSolv = solver_create(pTdnf->pSack->pPool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAllowErasing)
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

    nProblems = solver_solve(pSolv, pQueueJobs);
    if (nProblems > 0)
    {
        dwError = TDNFGetSkipProblemOption(pTdnf, &dwSkipProblem);
        BAIL_ON_TDNF_ERROR(dwError);
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
                  pTrans,
                  pSolv,
                  &pInfo,
                  pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppInfo = pInfo;

cleanup:
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
    TDNF_SAFE_FREE_MEMORY(pInfo);
    if(ppInfo)
    {
        *ppInfo = NULL;
    }
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

    Queue queueJobs = {0};
    int nAllowErasing = 0;
    char** ppszExcludes = NULL;
    uint32_t dwExcludeCount = 0;
    char **ppszAutoInstalled = NULL;

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

        for (int i = 0; i < pQueuePkgList->count; i++)
        {
            Id dwId = pQueuePkgList->elements[i];
            TDNFAddGoal(pTdnf, nAlterType, &queueJobs, dwId,
                        dwExcludeCount, ppszExcludes);
        }
    }

    nAllowErasing =
        pTdnf->pArgs->nAllowErasing ||
        nAlterType == ALTER_ERASE ||
        nAlterType == ALTER_AUTOERASE ||
        nAlterType == ALTER_AUTOERASEALL;
    if(nAllowErasing)
    {
        TDNFAddUserInstalledToJobs(pTdnf, &queueJobs);
        BAIL_ON_TDNF_ERROR(dwError);
        /* TODO: deal with no db error? */
    }

    dwError = TDNFSolv(pTdnf, &queueJobs, ppszExcludes, dwExcludeCount,
                       nAllowErasing,
                       (pTdnf->pConf->nCleanRequirementsOnRemove &&
                                !pTdnf->pArgs->nNoAutoRemove) ||
                               nAlterType == ALTER_AUTOERASE,
                       ppInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nAlterType == ALTER_INSTALL)
    {
        dwError = TDNFAddUserInstall(pTdnf, pQueuePkgList, *ppInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszAutoInstalled);
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    queue_free(&queueJobs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFHistoryGoal(
    PTDNF pTdnf,
    Queue *pqInstall,
    Queue *pqErase,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    )
{
    uint32_t dwError = 0;
    Queue queueJobs = {0};
    char** ppszExcludes = NULL;
    uint32_t dwExcludeCount = 0;

    if(!pTdnf || !ppInfo || !pqInstall || !pqErase)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPkgsToExclude(pTdnf, &dwExcludeCount, &ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_init(&queueJobs);

    for (int i = 0; i < pqInstall->count; i++)
    {
        Id id = pqInstall->elements[i];
        dwError = TDNFAddGoal(pTdnf, ALTER_INSTALL, &queueJobs, id,
                    dwExcludeCount, ppszExcludes);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for (int i = 0; i < pqErase->count; i++)
    {
        Id id = pqErase->elements[i];
        dwError = TDNFAddGoal(pTdnf, ALTER_ERASE, &queueJobs, id,
                    dwExcludeCount, ppszExcludes);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFSolv(pTdnf, &queueJobs, ppszExcludes, dwExcludeCount,
                       1, /* nAllowErasing */
                       0, /* nAutoErase */
                       ppInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAddUserInstall(pTdnf, pqInstall, *ppInfo);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    queue_free(&queueJobs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFAddUserInstall(
    PTDNF pTdnf,
    Queue* pQueueGoal,
    PTDNF_SOLVED_PKG_INFO ppInfo
    )
{
    uint32_t dwError = 0;
    int i;
    char **ppszPkgsUserInstall = NULL;

    if (!pTdnf || !pQueueGoal || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(pQueueGoal->count + 1,
                                 sizeof(char **),
                                 (void **)&ppszPkgsUserInstall);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 0; i < pQueueGoal->count; i++)
    {
        dwError = SolvGetPkgNameFromId(
                       pTdnf->pSack,
                       pQueueGoal->elements[i],
                       &ppszPkgsUserInstall[i]);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppInfo->ppszPkgsUserInstall = ppszPkgsUserInstall;
cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(ppszPkgsUserInstall);
    goto cleanup;
}

uint32_t
TDNFMarkAutoInstalledSinglePkg(
    PTDNF pTdnf,
    const char *pszPkgName
)
{
    uint32_t dwError = 0;
    int rc;
    struct history_ctx *pHistoryCtx = NULL;

    if (!pTdnf || !pszPkgName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetHistoryCtx(pTdnf, &pHistoryCtx, 1);
    BAIL_ON_TDNF_ERROR(dwError);

    rc = history_set_auto_flag(pHistoryCtx, pszPkgName, 0);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    if (pHistoryCtx)
    {
        destroy_history_ctx(pHistoryCtx);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFMarkAutoInstalled(
    PTDNF pTdnf,
    struct history_ctx *pHistoryCtx,
    PTDNF_SOLVED_PKG_INFO ppInfo,
    int nAutoOnly
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if (!pTdnf || !pHistoryCtx || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* ppInfo->pPkgsToInstall contains packages that were installed.
       ppInfo->ppszPkgsUserInstall contains packages that the user intended to
       install. Therefore, any packages that are in pPkgsToInstall but not in
       ppszPkgsUserInstall are automatic installs and were pulled in by
       dependencies.

       Corner cases:
       - packages that are dependencies but are already installed will be
         unaffected
       - on upgrades/downgrades, only additional packages will be in
         pPkgsToInstall. These are automatic if the upgrade was invoked w/out
         package args. If they are in package args, they are not in pPkgsToInstall
         (but will be in pPkgsToUpgrade) and their status will not change.
    */
    for (pPkgInfo = ppInfo->pPkgsToInstall; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        int rc;
        const char *pszName = pPkgInfo->pszName;
        int nFlag = 1;
        /* check if user installed */
        if (ppInfo->ppszPkgsUserInstall)
        {
            /* TODO: if both lists were sorted, we could start with i
               where it left last time */
            for (int i = 0; ppInfo->ppszPkgsUserInstall[i]; i++)
            {
                if (strcmp(pszName,
                           ppInfo->ppszPkgsUserInstall[i]) == 0)
                {
                    nFlag = 0;
                    break;
                }
            }
        }
        if (!nAutoOnly || nFlag == 1)
        {
            rc = history_set_auto_flag(pHistoryCtx, pszName, nFlag);
            if (rc != 0)
            {
                dwError = ERROR_TDNF_HISTORY_ERROR;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }
cleanup:
    return dwError;
error:
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
        case ALTER_AUTOERASE:
        case ALTER_AUTOERASEALL:
            dwError = SolvAddPkgEraseJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        case ALTER_REINSTALL:
        case ALTER_INSTALL:
        case ALTER_UPGRADE:
            dwError = SolvAddPkgInstallJob(pQueueJobs, dwId);
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

uint32_t
TDNFSolvAddPkgLocks(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    )
{
    uint32_t dwError = 0;
    char **ppszPackages = NULL;
    int i;

    if(!pTdnf || !pQueueJobs || !pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszPackages = pTdnf->pConf->ppszPkgLocks;

    for (i = 0; ppszPackages && ppszPackages[i]; i++)
    {
        char *pszPkg = ppszPackages[i];
        Id idPkg = pool_str2id(pPool, pszPkg, 1);
        if (idPkg)
        {
            Id p;
            Solvable *s;
            FOR_REPO_SOLVABLES(pPool->installed, p, s)
            {
                if (idPkg == s->name)
                {
                    queue_push2(pQueueJobs, SOLVER_SOLVABLE_NAME|SOLVER_LOCK, idPkg);
                    break;
                }
            }
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFSolvAddMinVersions(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    )
{
    uint32_t dwError = 0;
    char **ppszPackages = NULL;
    char **ppszTokens = NULL;
    Map *pMapMinVersions = NULL;
    char *pszTmp = NULL;

    if(!pTdnf || !pQueueJobs || !pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszPackages = pTdnf->pConf->ppszMinVersions;
    if (!ppszPackages)
    {
        goto cleanup;
    }

    dwError = TDNFAllocateMemory(
                          1,
                          sizeof(Map),
                          (void**)&pMapMinVersions);
    BAIL_ON_TDNF_ERROR(dwError);

    map_init(pMapMinVersions, pPool->nsolvables);

    for (int i = 0; ppszPackages && ppszPackages[i]; i++)
    {
        char *pszPkg = ppszPackages[i];

        dwError = TDNFAllocateString(pszPkg, &pszTmp);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSplitStringToArray(pszTmp, "=", &ppszTokens);
        BAIL_ON_TDNF_ERROR(dwError);

        if (ppszTokens[0] && ppszTokens[1]) {
            Dataiterator di;

            dwError = dataiterator_init(&di, pPool, 0, 0, SOLVABLE_NAME, ppszTokens[0], SEARCH_STRING);
            BAIL_ON_TDNF_ERROR(dwError);

            while (dataiterator_step(&di))
            {
                Solvable *pSolv = pool_id2solvable(pPool, di.solvid);
                const char *pszEvr = solvable_lookup_str(pSolv, SOLVABLE_EVR);
                if (pool_evrcmp_str( pPool, pszEvr, ppszTokens[1], EVRCMP_COMPARE) < 0)
                {
                    MAPSET(pMapMinVersions, di.solvid);
                }
            }
            dataiterator_free(&di);
        }
    }

    if (!pPool->considered)
    {
        dwError = TDNFAllocateMemory(
                             1,
                             sizeof(Map),
                             (void**)&pPool->considered);
        map_init(pPool->considered, pPool->nsolvables);
    }
    else
    {
        map_grow(pPool->considered, pPool->nsolvables);
    }

    map_setall(pPool->considered);
    map_subtract(pPool->considered, pMapMinVersions);
cleanup:
    if(pMapMinVersions)
    {
        map_free(pMapMinVersions);
        TDNFFreeMemory(pMapMinVersions);
    }
    TDNF_SAFE_FREE_MEMORY(pszTmp);
    TDNF_SAFE_FREE_STRINGARRAY(ppszTokens);
    return dwError;
error:
    goto cleanup;
}

