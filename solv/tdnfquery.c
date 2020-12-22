/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
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
    while (ppszTmpNames && *ppszTmpNames)
    {
        dwPkgs++;
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
        TDNFFreeStringArray(pQuery->ppszPackageNames);
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
    PSolvQuery pQuery,
    uint32_t dwSelectFlags
    )
{
    uint32_t dwError = 0;
    char** ppszPkgNames = NULL;
    Pool *pPool = NULL;
    Queue queueJob = {0};
    uint32_t nFlags = 0;
    uint32_t nRetFlags = 0;

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
            nFlags  = dwSelectFlags;
            nRetFlags = 0;

            queue_empty(&queueJob);
            if (!pPool || !pPool->solvables || !pPool->whatprovides)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
            }

            nRetFlags = selection_make(
                         pPool,
                         &queueJob,
                         *ppszPkgNames,
                         nFlags);

            if (pQuery->queueRepoFilter.count)
            {
                selection_filter(pPool, &queueJob, &pQuery->queueRepoFilter);
            }
            if (!queueJob.count)
            {
                nFlags |= SELECTION_NOCASE;
                nRetFlags = selection_make(
                                pPool,
                                &queueJob,
                                *ppszPkgNames,
                                nFlags);
                if (pQuery->queueRepoFilter.count)
                {
                    selection_filter(
                        pPool,
                        &queueJob,
                        &pQuery->queueRepoFilter);
                }
                if (queueJob.count)
                {
                    pr_info("[ignoring case for '%s']\n", *ppszPkgNames);
                }
            }
            if (queueJob.count)
            {
                if (nRetFlags & SELECTION_FILELIST)
                {
                    pr_info("[using file list match for '%s']\n",
                           *ppszPkgNames);
                }
                if (nRetFlags & SELECTION_PROVIDES)
                {
                    pr_info("[using capability match for '%s']\n",
                           *ppszPkgNames);
                }
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
        {
            selection_filter(pPool, &queueJob, &pQuery->queueRepoFilter);
        }
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
SolvApplyUpDownScope(
    PSolvQuery pQuery,
    int nUp
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pQuery->ppszPackageNames)
    {
        dwError = SolvFindAllInstalled(pQuery->pSack, &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = SolvFindInstalledPkgByMultipleNames(
                      pQuery->pSack,
                      pQuery->ppszPackageNames,
                      &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvFindAllUpDownCandidates(
                  pQuery->pSack,
                  pInstalledPkgList,
                  nUp,
                  &pQuery->queueResult);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
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

static inline int
is_pseudo_package(Pool *pool, Solvable *s)
{
    const char *n = pool_id2str(pool, s->name);
    if (*n == 'p' && !strncmp(n, "patch:", 6))
    {
        return 1;
    }
    return 0;
}

uint32_t
SolvApplyListQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    Queue queueTmp = {0};
    uint32_t nFlags = 0;

    if(!pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    queue_init(&queueTmp);
    nFlags = SELECTION_NAME | SELECTION_PROVIDES | SELECTION_GLOB;
    nFlags |= SELECTION_CANON|SELECTION_DOTARCH|SELECTION_REL;

    dwError = SolvGenerateCommonJob(pQuery, nFlags);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if(pQuery->nScope == SCOPE_UPGRADES)
    {
        dwError = SolvApplyUpDownScope(pQuery, 1);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pQuery->nScope == SCOPE_DOWNGRADES)
    {
        dwError = SolvApplyUpDownScope(pQuery, 0);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pQuery->queueJob.count > 0)
    {
        for (nIndex = 0; nIndex < pQuery->queueJob.count ; nIndex += 2)
        {
            queue_empty(&queueTmp);
            Id p = 0, pp = 0, how = 0, what = 0;
            what = pQuery->queueJob.elements[nIndex + 1];
            how = SOLVER_SELECTMASK & pQuery->queueJob.elements[nIndex];
            Pool *pool = pQuery->pSack->pPool;
            if (how == SOLVER_SOLVABLE_ALL)
            {
               FOR_POOL_SOLVABLES(p)
               {
                  if(is_pseudo_package(pool, &pool->solvables[p]))
                     continue;
                  queue_push(&queueTmp, p);
               }
            }
            else if (how == SOLVER_SOLVABLE_REPO)
            {
               Repo *repo = pool_id2repo(pool, what);
               if (repo)
               {
                   Solvable *s = NULL;

	           FOR_REPO_SOLVABLES(repo, p, s)
                   {
                      if (is_pseudo_package(pool, &pool->solvables[p]))
                          continue;
	              queue_push(&queueTmp, p);
                   }
	       }
            }
            else
            {
               FOR_JOB_SELECT(p, pp, how, what)
               {
                  if (is_pseudo_package(pool, &pool->solvables[p]))
                      continue;
	          queue_push(&queueTmp, p);
               }
            }
            queue_insertn(&pQuery->queueResult,
                          pQuery->queueResult.count,
                          queueTmp.count,
                          queueTmp.elements);
        }
    }
    else if(!pQuery->ppszPackageNames ||
            IsNullOrEmptyString(pQuery->ppszPackageNames[0]))
    {
        Id p = 0;
        Pool *pool = pQuery->pSack->pPool;
        FOR_POOL_SOLVABLES(p)
        {
            if(is_pseudo_package(pool, &pool->solvables[p]))
                continue;
            queue_push(&pQuery->queueResult, p);
        }
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
    uint32_t nFlags = 0;

    nFlags = SELECTION_NAME | SELECTION_PROVIDES | SELECTION_GLOB;
    nFlags |= SELECTION_CANON|SELECTION_DOTARCH|SELECTION_REL;

    dwError = SolvGenerateCommonJob(pQuery, nFlags);
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

    for(nIndex = dwStartIndex; nIndex < dwEndIndex; nIndex++)
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

uint32_t
SolvApplyProvidesQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    Queue queueTmp = {0};
    uint32_t nFlags = 0;

    if(!pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    queue_init(&queueTmp);
    nFlags = SELECTION_FILELIST | SELECTION_NAME | SELECTION_GLOB |
             SELECTION_PROVIDES;
    nFlags |= SELECTION_CANON| SELECTION_FILELIST|SELECTION_REL;

    dwError = SolvGenerateCommonJob(pQuery, nFlags);
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
SolvGetUpdateAdvisories(
    PSolvSack pSack,
    Id dwPkgIdpkg,
    PSolvPackageList* ppPkgList)
{
    uint32_t dwError = 0;
    Dataiterator di = {0};
    Id dwEvr = 0;
    Id dwArch = 0;
    int nCompareResult = 0;
    Queue queueAdv = {0};
    PSolvPackageList pPkgList = NULL;
    Solvable *pSolvable = NULL;
    const char* pszPkgName = NULL;

    if(!pSack || !pSack->pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&queueAdv);
    pSolvable = pool_id2solvable(pSack->pPool, dwPkgIdpkg);
    if(!pSolvable)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszPkgName = pool_id2str(pSack->pPool, pSolvable->name);

    dataiterator_init(&di,
                      pSack->pPool,
                      0,
                      0,
                      UPDATE_COLLECTION_NAME,
                      pszPkgName,
                      SEARCH_STRING);

    dataiterator_prepend_keyname(&di, UPDATE_COLLECTION);
    while (dataiterator_step(&di))
    {
        dataiterator_setpos_parent(&di);
        dwArch = pool_lookup_id(
                     pSack->pPool,
                     SOLVID_POS,
                     UPDATE_COLLECTION_ARCH);
        if (dwArch!= pSolvable->arch)
            continue;
        dwEvr = pool_lookup_id(
                    pSack->pPool,
                    SOLVID_POS,
                    UPDATE_COLLECTION_EVR);

        if (!dwEvr)
            continue;

        nCompareResult = pool_evrcmp(
                             pSack->pPool,
                             dwEvr,
                             pSolvable->evr,
                             EVRCMP_COMPARE);
        if (nCompareResult > 0 )
        {
            queue_push(&queueAdv, di.solvid);
            dataiterator_skip_solvable(&di);
        }
    }

    dwError = SolvQueueToPackageList(&queueAdv, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgList = pPkgList;

cleanup:
    dataiterator_free(&di);
    queue_free(&queueAdv);
    return dwError;

error:
    if(ppPkgList)
    {
        *ppPkgList = NULL;
    }
    goto cleanup;
}

uint32_t
SolvFindUpDownCandidateForSinglePkg(
    PSolvSack pSack,
    Queue* pQueueCandidate,
    Id dwPkgId,
    int nUp
    )
{
    uint32_t dwError = 0;
    char* pszName = NULL;
    uint32_t dwCount = 0;
    uint32_t dwPkgIndex = 0;
    Id dwCandidateId = 0;
    int dwEvrCompare = 0;
    PSolvPackageList pUpDownCandidates = NULL;

    if(!pSack || !pQueueCandidate)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgNameFromId(pSack, dwPkgId, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAvailablePkgByName(
                  pSack,
                  pszName,
                  &pUpDownCandidates);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pUpDownCandidates, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwCount; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(
                      pUpDownCandidates,
                      dwPkgIndex,
                      &dwCandidateId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvCmpEvr(
                      pSack,
                      dwCandidateId,
                      dwPkgId,
                      &dwEvrCompare);
        BAIL_ON_TDNF_ERROR(dwError);

        if((nUp && dwEvrCompare > 0) || (!nUp && dwEvrCompare < 0))
        {
            queue_push(pQueueCandidate, dwCandidateId);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    if(pUpDownCandidates)
    {
        SolvFreePackageList(pUpDownCandidates);
    }

    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvFindAllUpDownCandidates(
    PSolvSack pSack,
    PSolvPackageList  pInstalledPackages,
    int up,
    Queue *pQueueResult
    )
{
    uint32_t dwError = 0;
    uint32_t dwSize  = 0;
    uint32_t dwPkgIndex = 0;
    Queue queueUpDown = {0};
    Id dwPkgId = 0;

    if(!pSack ||
       !pSack->pPool ||
       !pInstalledPackages ||
       !pQueueResult)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&queueUpDown);

    dwError = SolvGetPackageListSize(pInstalledPackages, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pInstalledPackages, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvFindUpDownCandidateForSinglePkg(
                      pSack,
                      &queueUpDown,
                      dwPkgId,
                      up);
        if(dwError == ERROR_TDNF_NO_MATCH)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_insertn(pQueueResult,
                  pQueueResult->count,
                  queueUpDown.count,
                  queueUpDown.elements);
cleanup:
    return dwError;

error:
    goto cleanup;
}
