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
    Transaction*    pTrans,
    PTDNF           pTdnf, 
    PTDNF_PKG_INFO* pPkgInfo,
    Id              dwType)
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PSolvPackageList pPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack|| !pTrans || !pPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetTransResultsWithType(pTrans, dwType, pPkgList);
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
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                SOLVER_TRANSACTION_INSTALL);
}

uint32_t
TDNFGetReinstallPackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                SOLVER_TRANSACTION_REINSTALL);
}

uint32_t
TDNFGetUpgradePackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                SOLVER_TRANSACTION_UPGRADE);
}

uint32_t
TDNFGetErasePackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                SOLVER_TRANSACTION_ERASE);
}

uint32_t
TDNFGetObsoletedPackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    return TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                SOLVER_TRANSACTION_OBSOLETED);
}

uint32_t
TDNFGetUnneededPackages(
    Solver*         pSolv,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo)
{
    uint32_t dwError = 0;
    PSolvPackageList pPkgList = NULL;
    Queue queueResult;
    queue_init(&queueResult);

    if(!pTdnf || !pTdnf->pSack|| !pSolv || !pPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    solver_get_unneeded(pSolv, &queueResult, 0);

    if(queueResult.count > 0)
    {
        dwError = SolvCreatePackageList(&pPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvQueueToPackageList(&queueResult, pPkgList);
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
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    PTDNF_PKG_INFO* pRemovePkgInfo)
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    Id dwInstalledId = 0;
    PSolvPackageList pRemovePkgList = NULL;
    PTDNF_PKG_INFO   pInfo = NULL;
    Queue queuePkgToRemove;
    queue_init(&queuePkgToRemove);

    if(!pTdnf || !pTdnf->pSack|| !pTrans || !pPkgInfo || !pRemovePkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetPackagesWithSpecifiedType(pTrans, pTdnf, pPkgInfo,
                    SOLVER_TRANSACTION_DOWNGRADE);
    BAIL_ON_TDNF_ERROR(dwError);
    pInfo = *pPkgInfo;
    if(!pInfo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    while(pInfo)
    {
        SolvEmptyPackageList(pInstalledPkgList);
        dwError = SolvFindInstalledPkgByName(pTdnf->pSack,
                        pInfo->pszName,
                        pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        SolvGetPackageId(pInstalledPkgList, 0, &dwInstalledId);
        queue_push(&queuePkgToRemove, dwInstalledId);
        pInfo = pInfo->pNext;
    }

    if(queuePkgToRemove.count > 0)
    {
        dwError = SolvCreatePackageList(&pRemovePkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvQueueToPackageList(&queuePkgToRemove, pRemovePkgList);
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
TDNFGoal(
    PTDNF                   pTdnf,
    Queue*                  pQueuePkgList,
    PTDNF_SOLVED_PKG_INFO   pInfo
    )
{
    uint32_t dwError = 0;

    PTDNF_SOLVED_PKG_INFO pInfoTemp = NULL;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    Queue queueJobs;
    queue_init(&queueJobs);

    int nFlags = 0;
    int i = 0;

    if(!pTdnf || !pInfo || !pQueuePkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pInfo->nAlterType == ALTER_UPGRADEALL)
    {
        dwError = SolvAddUpgradeAllJob(&queueJobs);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pInfo->nAlterType == ALTER_DISTRO_SYNC)
    {
        dwError = SolvAddDistUpgradeJob(&queueJobs);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        for(i = 0; i < pQueuePkgList->count; i++)
        {
            Id id = pQueuePkgList->elements[i];
            TDNFAddGoal(pTdnf, pInfo->nAlterType, &queueJobs, id);
        }
    }

    if(pTdnf->pArgs->nBest)
    {
        nFlags = nFlags | SOLVER_FORCEBEST;
    }
    dwError = SolvAddFlagsToJobs(&queueJobs, nFlags);

    if(pTdnf->pArgs->nDebugSolver)
    {
//        Report debug info here
    }

    pSolv = solver_create(pTdnf->pSack->pPool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nAllowErasing ||
       pInfo->nAlterType == ALTER_ERASE ||
       pInfo->nAlterType == ALTER_AUTOERASE)
    {
        solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_UNINSTALL, 1);
    }
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_VENDORCHANGE, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_KEEP_ORPHANS, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_YUM_OBSOLETES, 1);

    if (solver_solve(pSolv, &queueJobs) == 0)
    {
        pTrans = solver_create_transaction(pSolv);
    }

    dwError = TDNFGoalGetAllResultsIgnoreNoData(
                  pInfo->nAlterType,
                  pTrans,
                  pSolv,
                  &pInfoTemp,
                  pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    pInfo->pPkgsToInstall = pInfoTemp->pPkgsToInstall;
    pInfo->pPkgsToUpgrade = pInfoTemp->pPkgsToUpgrade;
    pInfo->pPkgsToDowngrade = pInfoTemp->pPkgsToDowngrade;
    pInfo->pPkgsToRemove = pInfoTemp->pPkgsToRemove;
    pInfo->pPkgsUnNeeded = pInfoTemp->pPkgsUnNeeded;
    pInfo->pPkgsToReinstall = pInfoTemp->pPkgsToReinstall;
    pInfo->pPkgsObsoleted = pInfoTemp->pPkgsObsoleted;
    pInfo->pPkgsRemovedByDowngrade = pInfoTemp->pPkgsRemovedByDowngrade;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pInfoTemp);
    return dwError;

error:
    queue_free(&queueJobs);
    if(pTrans)
    {
        transaction_free(pTrans);
    }
    if(pSolv)
    {
        solver_free(pSolv);
    }
    goto cleanup;
}


uint32_t
TDNFAddGoal(
    PTDNF   pTdnf,
    int     nAlterType,
    Queue*  pQueueJobs,
    Id      dwId
    )
{
    uint32_t dwError = 0;
    char* pszPkg = NULL;
    int flags = 0;
    int i = 0;
    Queue queueJob;
    queue_init(&queueJob);

    if(!pQueueJobs || dwId == 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
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
            dwError = SolvAddPkgInstallJob(pQueueJobs, dwId);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        case ALTER_UPGRADE:
            dwError = SolvGetPkgNevrFromId(pTdnf->pSack, dwId, &pszPkg);
            BAIL_ON_TDNF_ERROR(dwError);

            flags = SELECTION_NAME|SELECTION_PROVIDES|SELECTION_GLOB;
            flags |= SELECTION_CANON|SELECTION_DOTARCH|SELECTION_REL;
            selection_make(pTdnf->pSack->pPool, &queueJob, pszPkg, flags);
            for (i = 0; i < queueJob.count; i += 2)
            {
                queue_push2(pQueueJobs,
                    queueJob.elements[i],
                    queueJob.elements[i + 1]);
            }
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
    queue_free(&queueJob);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGoalGetAllResultsIgnoreNoData(
    int                     nResolveFor,
    Transaction*            pTrans,
    Solver*                 pSolv,
    PTDNF_SOLVED_PKG_INFO*  ppInfo,
    PTDNF                   pTdnf
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

