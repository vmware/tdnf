/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

#define MODE_INSTALL     1
#define MODE_ERASE       2
#define MODE_UPDATE      3
#define MODE_DISTUPGRADE 4
#define MODE_VERIFY      5
#define MODE_PATCH       6

uint32_t
SolvAddUpgradeAllJob(
    Queue* pQueueJobs
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_UPDATE|SOLVER_SOLVABLE_ALL, 0);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddDistUpgradeJob(
    Queue* pQueueJobs
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_DISTUPGRADE|SOLVER_SOLVABLE_ALL, 0);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddFlagsToJobs(
    Queue* pQueueJobs,
    int nFlags
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    for (int i = 0; i < pQueueJobs->count; i += 2)
    {
        pQueueJobs->elements[i] |= nFlags;
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddPkgInstallJob(
    Queue*  pQueueJobs,
    Id      dwId
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_SOLVABLE|SOLVER_INSTALL, dwId);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddPkgDowngradeJob(
    Queue*  pQueueJobs,
    Id      dwId
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_SOLVABLE|SOLVER_INSTALL, dwId);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddPkgEraseJob(
    Queue*  pQueueJobs,
    Id      dwId
    )
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_SOLVABLE|SOLVER_ERASE, dwId);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddPkgUserInstalledJob(
    Queue*  pQueueJobs,
    Id      dwId)
{
    uint32_t dwError = 0;
    if(!pQueueJobs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_push2(pQueueJobs, SOLVER_SOLVABLE|SOLVER_USERINSTALLED, dwId);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCreateQuery(
    PSolvSack   pSack,
    PSolvQuery* ppQuery
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack || !ppQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(SolvQuery), (void **)&pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    pQuery->pSack = pSack;
    queue_init(&pQuery->queueJob);
    queue_init(&pQuery->queueRepoFilter);
    queue_init(&pQuery->queueResult);
    *ppQuery = pQuery;

cleanup:
    return dwError;

error:
    if(ppQuery)
    {
        *ppQuery = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pQuery);
    goto cleanup;
}

void
SolvFreeQuery(
    PSolvQuery pQuery
    )
{
    if(pQuery)
    {
        if(pQuery->pTrans)
        {
            transaction_free(pQuery->pTrans);
        }

        if(pQuery->pSolv)
        {
            solver_free(pQuery->pSolv);
        }

        queue_free(&pQuery->queueJob);
        queue_free(&pQuery->queueRepoFilter);
        queue_free(&pQuery->queueResult);
        if(pQuery->ppszPackageNames)
        {
            TDNFFreeStringArray(pQuery->ppszPackageNames);
        }
        TDNF_SAFE_FREE_MEMORY(pQuery);
    }
}

uint32_t
SolvApplySinglePackageFilter(
    PSolvQuery pQuery,
    const char* pszPackageName
    )
{
    uint32_t dwError = 0;
    char** ppCopyOfpkgNames = NULL;
    if(!pQuery || !pszPackageName || IsNullOrEmptyString(pszPackageName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  2,
                  sizeof(char*),
                  (void**)&ppCopyOfpkgNames);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(
                  pszPackageName,
                  &ppCopyOfpkgNames[0]);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pQuery->ppszPackageNames)
    {
        TDNFFreeStringArray(pQuery->ppszPackageNames);
    }

    pQuery->ppszPackageNames = ppCopyOfpkgNames;

cleanup:
    return dwError;

error:
    if(ppCopyOfpkgNames)
    {
        TDNFFreeStringArray(ppCopyOfpkgNames);
    }
    goto cleanup;
}

uint32_t
SolvApplyPackageFilter(
    PSolvQuery pQuery,
    char** ppszPackageNames
    )
{
    uint32_t dwError = 0;
    int     dwPkgs = 0;
    char** ppszTmpNames = NULL;
    char** ppszCopyOfPkgNames = NULL;
    int nIndex = 0;

    if(!pQuery || !ppszPackageNames)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszTmpNames = ppszPackageNames;
    while(*ppszTmpNames)
    {
        if(!IsNullOrEmptyString(ppszTmpNames))
        {
            dwPkgs++;
        }
        ppszTmpNames++;
    }

    if(dwPkgs == 0)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  dwPkgs + 1,
                  sizeof(char*),
                  (void**)&ppszCopyOfPkgNames);
    BAIL_ON_TDNF_ERROR(dwError);

    for(nIndex = 0; nIndex < dwPkgs; ++nIndex)
    {
        if(!IsNullOrEmptyString(ppszPackageNames))
        {
            dwError = TDNFAllocateString(
                          *ppszPackageNames,
                          &ppszCopyOfPkgNames[nIndex]);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        ppszPackageNames++;
    }

    if(pQuery->ppszPackageNames)
    {
        TDNFFreeStringArray(ppszCopyOfPkgNames);
    }
    pQuery->ppszPackageNames = ppszCopyOfPkgNames;

cleanup:
  
    return dwError;
error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    if(ppszCopyOfPkgNames)
    {
        TDNFFreeStringArray(ppszCopyOfPkgNames);
    }
    goto cleanup;
}

uint32_t
SolvAddSystemRepoFilter(
    PSolvQuery  pQuery
    )
{
    uint32_t dwError = 0;
    Pool *pool = NULL;

    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pool = pQuery->pSack->pPool;
    queue_push2(&pQuery->queueRepoFilter,
                SOLVER_SOLVABLE_REPO | SOLVER_SETREPO,
                pool->installed->repoid);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvAddAvailableRepoFilter(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    Pool *pool = NULL;
    int i = 0;

    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pool = pQuery->pSack->pPool;
    FOR_REPOS(i, pRepo)
    {
        if (strcasecmp(SYSTEM_REPO_NAME, pRepo->name))
        {
            queue_push2(
                &pQuery->queueRepoFilter,
                SOLVER_SOLVABLE_REPO | SOLVER_SETREPO | SOLVER_SETVENDOR,
                pRepo->repoid);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGenerateCommonJob(
    PSolvQuery  pQuery
    )
{
    uint32_t dwError = 0;
    char** ppszPkgNames = NULL;
    Pool *pPool = NULL;
    Queue queueJob = {0};

    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    ppszPkgNames = pQuery->ppszPackageNames;
    queue_init(&queueJob);
    pPool = pQuery->pSack->pPool;
    if(ppszPkgNames)
    {
        while(*ppszPkgNames)
        {
            int flags = 0;
            int rflags = 0;

            queue_empty(&queueJob);
            flags = SELECTION_NAME|SELECTION_PROVIDES|SELECTION_GLOB;
            flags |= SELECTION_CANON|SELECTION_DOTARCH|SELECTION_REL;
                rflags = selection_make(
                             pPool,
                             &queueJob,
                             *ppszPkgNames,
                             flags);
            if (pQuery->queueRepoFilter.count)
                selection_filter(pPool, &queueJob, &pQuery->queueRepoFilter);
            if (!queueJob.count)
            {
                flags |= SELECTION_NOCASE;
                rflags = selection_make(
                             pPool,
                             &queueJob,
                             *ppszPkgNames,
                             flags);
                if (pQuery->queueRepoFilter.count)
                    selection_filter(
                        pPool,
                        &queueJob,
                        &pQuery->queueRepoFilter);
                if (queueJob.count)
                    printf("[ignoring case for '%s']\n", *ppszPkgNames);
            }
            if (queueJob.count)
            {
                if (rflags & SELECTION_FILELIST)
                    printf("[using file list match for '%s']\n",
                           *ppszPkgNames);
                if (rflags & SELECTION_PROVIDES)
                    printf("[using capability match for '%s']\n",
                           *ppszPkgNames);
                queue_insertn(&pQuery->queueJob,
                              pQuery->queueJob.count,
                              queueJob.count,
                              queueJob.elements);
            }
            ppszPkgNames++;
        }
    }
    else if(pQuery->queueRepoFilter.count)
    {
        queue_empty(&queueJob);
        queue_push2(&queueJob, SOLVER_SOLVABLE_ALL, 0);
        if (pQuery->queueRepoFilter.count)
            selection_filter(pPool, &queueJob, &pQuery->queueRepoFilter);
        queue_insertn(&pQuery->queueJob,
                        pQuery->queueJob.count,
                        queueJob.count,
                        queueJob.elements);
    }

cleanup:
    queue_free(&queueJob);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvRunSolv(
    PSolvQuery pQuery,
    uint32_t dwMainMode,
    uint32_t dwMode,
    Queue* queueJobs,
    Solver** ppSolv)
{
    uint32_t dwError = 0;
    int nJob = 0;
    Solver *pSolv = NULL;

    if(!queueJobs->count && (dwMainMode == MODE_UPDATE || 
       dwMainMode == MODE_DISTUPGRADE ||
       dwMainMode == MODE_VERIFY))
    {
        queue_push2(queueJobs, SOLVER_SOLVABLE_ALL, 0);
    }

    for (nJob = 0; nJob < queueJobs->count; nJob += 2)
    {
        queueJobs->elements[nJob] |= dwMode;
        if (dwMode == SOLVER_UPDATE && 
            pool_isemptyupdatejob(
                pQuery->pSack->pPool,
                queueJobs->elements[nJob],
                queueJobs->elements[nJob + 1]))
        {
            queueJobs->elements[nJob] ^= SOLVER_UPDATE ^ SOLVER_INSTALL;
        }
    }
    pSolv = solver_create(pQuery->pSack->pPool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_SPLITPROVIDES, 1);
    if (dwMainMode == MODE_ERASE)
    {
        solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_UNINSTALL, 1);
    }

    if(solver_solve(pSolv, queueJobs))
    {
        dwError = ERROR_TDNF_SOLV_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppSolv = pSolv;
cleanup:
    return dwError;

error:
    if(ppSolv)
    {
        *ppSolv = NULL;
    }
    if(pSolv)
    {
        solver_free(pSolv);
    }
    goto cleanup;

}


uint32_t
SolvApplyListQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    Queue queueTmp = {0};

    if(!pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    queue_init(&queueTmp);

    dwError = SolvGenerateCommonJob(pQuery);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if(pQuery->queueJob.count > 0)
    {
        for (nIndex = 0; nIndex < pQuery->queueJob.count ; nIndex += 2)
        {
            queue_empty(&queueTmp);
            pool_job2solvables(pQuery->pSack->pPool, &queueTmp,
                               pQuery->queueJob.elements[nIndex],
                               pQuery->queueJob.elements[nIndex + 1]);
            queue_insertn(&pQuery->queueResult,
                          pQuery->queueResult.count,
                          queueTmp.count,
                          queueTmp.elements);
        }
    }
    else if(!pQuery->ppszPackageNames ||
            IsNullOrEmptyString(pQuery->ppszPackageNames[0]))
    {
        pool_job2solvables(pQuery->pSack->pPool,
                           &pQuery->queueResult,
                           SOLVER_SOLVABLE_ALL,
                           0);
    }

cleanup:
    queue_free(&queueTmp);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvApplyAlterQuery(
    PSolvQuery pQuery,
    uint32_t dwMainMode,
    uint32_t dwMode
    )
{
    uint32_t dwError = 0;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;

    dwError = SolvGenerateCommonJob(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvRunSolv(
                  pQuery,
                  dwMainMode,
                  dwMode,
                  &pQuery->queueJob,
                  &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);

    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->queueResult,
                  pQuery->queueResult.count,
                  pTrans->steps.count,
                  pTrans->steps.elements);

cleanup:
    if(pSolv)
        solver_free(pSolv);
    return dwError;

error:
    if(pTrans)
        transaction_free(pTrans);
    goto cleanup;
}

uint32_t
SolvApplyDistroSyncQuery(
    PSolvQuery pQuery
    )
{
    return SolvApplyAlterQuery(pQuery, MODE_DISTUPGRADE, SOLVER_DISTUPGRADE);
}

uint32_t
SolvApplySearch(
    PSolvQuery pQuery,
    char** ppszSearchStrings,
    int dwStartIndex,
    int dwEndIndex
    )
{
    Dataiterator di = {0};
    Pool* pPool = NULL;
    uint32_t dwError = 0;
    int nIndex = 0;
    Queue queueSel = {0};
    Queue queueResult = {0};

    if(!pQuery || !ppszSearchStrings)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&queueSel);
    queue_init(&queueResult);
    pPool = pQuery->pSack->pPool;
    pool_createwhatprovides(pPool);

    for(nIndex = dwStartIndex; nIndex < dwStartIndex; nIndex++)
    {
        queue_empty(&queueSel);
        queue_empty(&queueResult);
        dataiterator_init(&di, pPool, 0, 0, 0, ppszSearchStrings[nIndex],
                          SEARCH_SUBSTRING|SEARCH_NOCASE);
        dataiterator_set_keyname(&di, SOLVABLE_NAME);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&queueSel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_set_keyname(&di, SOLVABLE_SUMMARY);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&queueSel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_set_keyname(&di, SOLVABLE_DESCRIPTION);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&queueSel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_free(&di);

        selection_solvables(pPool, &queueSel, &queueResult);
        queue_insertn(&pQuery->queueResult,
                      pQuery->queueResult.count,
                      queueResult.count,
                      queueResult.elements);
    }

cleanup:
    queue_free(&queueSel);
    queue_free(&queueResult);
    return dwError;

error:
    goto cleanup;
}
