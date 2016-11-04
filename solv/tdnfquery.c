#include "includes.h"

#define MODE_INSTALL     1
#define MODE_ERASE       2
#define MODE_UPDATE      3
#define MODE_DISTUPGRADE 4
#define MODE_VERIFY      5
#define MODE_PATCH       6

uint32_t
SolvAddUpgradeAllJob(
    Queue* jobs
    )
{
    queue_push2(jobs, SOLVER_UPDATE|SOLVER_SOLVABLE_ALL, 0);
    return 0;
}

uint32_t
SolvAddDistUpgradeJob(
    Queue* jobs
    )
{
    queue_push2(jobs, SOLVER_DISTUPGRADE|SOLVER_SOLVABLE_ALL, 0);
    return 0;
}

uint32_t
SolvAddFlagsToJobs(
    Queue* jobs,
    int flags)
{
    for (int i = 0; i < jobs->count; i += 2)
    {
        jobs->elements[i] |= flags;//SOLVER_FORCEBEST;
    }
    return 0;
}

uint32_t
SolvAddPkgInstallJob(
    Queue* jobs,
    Id id)
{
    queue_push2(jobs, SOLVER_SOLVABLE|SOLVER_INSTALL, id);
    return 0;
}

uint32_t
SolvAddPkgDowngradeJob(
    Queue* jobs,
    Id id)
{
    queue_push2(jobs, SOLVER_SOLVABLE|SOLVER_INSTALL, id);
    return 0;
}

uint32_t
SolvAddPkgEraseJob(
    Queue* jobs,
    Id id)
{
    queue_push2(jobs, SOLVER_SOLVABLE|SOLVER_ERASE, id);
    return 0;
}

uint32_t
SolvAddPkgUserInstalledJob(
    Queue* jobs,
    Id id)
{
    queue_push2(jobs, SOLVER_SOLVABLE|SOLVER_USERINSTALLED, id);
    return 0;
}

uint32_t
SolvCreateQuery(
    PSolvSack pSack,
    PSolvQuery* ppQuery)
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pQuery = solv_calloc(1, sizeof(SolvQuery));
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pSolv = NULL;
    pQuery->pTrans = NULL;
    pQuery->ppszPackageNames = NULL;

    pQuery->pSack = pSack;
    queue_init(&pQuery->queueJob);
    queue_init(&pQuery->queueRepoFilter);
    queue_init(&pQuery->queueKindFilter);
    queue_init(&pQuery->queueArchFilter);
    queue_init(&pQuery->queueResult);
    *ppQuery = pQuery;
cleanup: 
    return dwError;

error:
    goto cleanup;
}

void
SolvFreeQuery(
    PSolvQuery pQuery)
{
    char** ppszTmpNames = NULL;
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
        queue_free(&pQuery->queueKindFilter);
        queue_free(&pQuery->queueArchFilter);
        queue_free(&pQuery->queueResult);
        if(pQuery->ppszPackageNames)
        {
            ppszTmpNames = pQuery->ppszPackageNames;
            while(*ppszTmpNames)
            {
                solv_free(*ppszTmpNames);
                ppszTmpNames++;
            }
            solv_free(pQuery->ppszPackageNames);
        }
        solv_free(pQuery);
    }
}

uint32_t
SolvApplySinglePackageFilter(
    PSolvQuery pQuery,
    const char* pszPackageName
    )
{
    uint32_t dwError = 0;
    char* pkgName = NULL;
    char** tmpNames = NULL;
    char** ppCopyOfpkgNames = NULL;
    if(!pQuery || !pszPackageName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppCopyOfpkgNames = solv_calloc(2, sizeof(char*));
    pkgName = solv_strdup(pszPackageName);
    if(!pkgName)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppCopyOfpkgNames = pkgName;
    pQuery->ppszPackageNames = ppCopyOfpkgNames;

cleanup:
  
    return dwError;
error:
    if(ppCopyOfpkgNames)
    {
        tmpNames = ppCopyOfpkgNames;
        while(*tmpNames)
        {
            solv_free(*tmpNames);
            tmpNames++;
        }
        solv_free(ppCopyOfpkgNames);
    }
    goto cleanup;
}

uint32_t
SolvApplyPackageFilter(
    PSolvQuery pQuery,
    char** ppszPackageNames)
{
    uint32_t dwError = 0;
    int pkgs = 0;
    char** tmpNames = NULL;
    char** ppCopyOfpkgNames = NULL;
    char* pkgName = NULL;
    if(!pQuery || !ppszPackageNames)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    tmpNames = ppszPackageNames;
    while(*tmpNames)
    {
        pkgs++;
        tmpNames++;
    }

    if (pkgs != 0)
    {
        ppCopyOfpkgNames = solv_calloc(pkgs + 1, sizeof(char*));
        tmpNames = ppCopyOfpkgNames;
        while(*ppszPackageNames)
        {
            pkgName = solv_strdup(*ppszPackageNames);
            if(!pkgName)
            {
                dwError = ERROR_TDNF_OUT_OF_MEMORY;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            *tmpNames = pkgName;
            tmpNames++;
            ppszPackageNames++;
        }
        pQuery->ppszPackageNames = ppCopyOfpkgNames;
    }
cleanup:
  
    return dwError;
error:
    if(ppCopyOfpkgNames)
    {
        tmpNames = ppCopyOfpkgNames;
        while(*tmpNames)
        {
            solv_free(*tmpNames);
            tmpNames++;
        }
        solv_free(ppCopyOfpkgNames);
    }

    goto cleanup;
}

uint32_t
SolvAddSystemRepoFilter(
    PSolvQuery pQuery
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
    queue_push2(&pQuery->queueRepoFilter, SOLVER_SOLVABLE_REPO | SOLVER_SETREPO,
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
            queue_push2(&pQuery->queueRepoFilter,
                        SOLVER_SOLVABLE_REPO | SOLVER_SETREPO | SOLVER_SETVENDOR,
                        pRepo->repoid);
        }
    }

cleanup: 
    return dwError;

error:
    goto cleanup;
}

static uint32_t
SolvGenerateCommonJob(
    PSolvQuery pQuery)
{
    uint32_t dwError = 0;
    char** pkgNames = pQuery->ppszPackageNames;
    Pool *pPool = NULL;
    Queue job2;
    queue_init(&job2);
    if(!pQuery || !pQuery->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    
    pPool = pQuery->pSack->pPool;
    if(pkgNames)
    {
        while(*pkgNames)
        {
            int flags = 0;
            int rflags = 0;

            queue_empty(&job2);
            flags = SELECTION_NAME|SELECTION_PROVIDES|SELECTION_GLOB;
            flags |= SELECTION_CANON|SELECTION_DOTARCH|SELECTION_REL;
                rflags = selection_make(pPool, &job2, *pkgNames, flags);
            if (pQuery->queueRepoFilter.count)
                selection_filter(pPool, &job2, &pQuery->queueRepoFilter);
            if (pQuery->queueArchFilter.count)
                selection_filter(pPool, &job2, &pQuery->queueArchFilter);
            if (pQuery->queueKindFilter.count)
                selection_filter(pPool, &job2, &pQuery->queueKindFilter);
            if (!job2.count)
            {
                flags |= SELECTION_NOCASE;

                rflags = selection_make(pPool, &job2, *pkgNames, flags);
                if (pQuery->queueRepoFilter.count)
                    selection_filter(pPool, &job2, &pQuery->queueRepoFilter);
                if (pQuery->queueArchFilter.count)
                    selection_filter(pPool, &job2, &pQuery->queueArchFilter);
                if (pQuery->queueKindFilter.count)
                    selection_filter(pPool, &job2, &pQuery->queueKindFilter);
                if (job2.count)
                    printf("[ignoring case for '%s']\n", *pkgNames);
            }
            if (job2.count)
            {
                if (rflags & SELECTION_FILELIST)
                    printf("[using file list match for '%s']\n", *pkgNames);
                if (rflags & SELECTION_PROVIDES)
                    printf("[using capability match for '%s']\n", *pkgNames);
                queue_insertn(&pQuery->queueJob, pQuery->queueJob.count, job2.count, job2.elements);
            }
            pkgNames++;
        }
    }
    else if(pQuery->queueRepoFilter.count || 
            pQuery->queueArchFilter.count || 
            pQuery->queueKindFilter.count)
    {
        queue_empty(&job2);
        queue_push2(&job2, SOLVER_SOLVABLE_ALL, 0);
        if (pQuery->queueRepoFilter.count)
            selection_filter(pPool, &job2, &pQuery->queueRepoFilter);
        if (pQuery->queueArchFilter.count)
            selection_filter(pPool, &job2, &pQuery->queueArchFilter);
        if (pQuery->queueKindFilter.count)
            selection_filter(pPool, &job2, &pQuery->queueKindFilter);
        queue_insertn(&pQuery->queueJob, pQuery->queueJob.count, job2.count, job2.elements);
    }


cleanup: 
    queue_free(&job2);
    return dwError;

error:
    goto cleanup;
}

static uint32_t
SolvRunSolv(
    PSolvQuery pQuery,
    uint32_t mainMode,
    uint32_t mode,
    Queue* jobs,
    Solver** ppSolv)
{
    uint32_t dwError = 0;
    int nJob = 0;
    Id problem = 0;
    Id solution = 0;
    int nProblemCount = 0;
    int nSolutionCount = 0;

    Solver *pSolv = NULL;

    if (!jobs->count && (mainMode == MODE_UPDATE || mainMode == MODE_DISTUPGRADE || mainMode == MODE_VERIFY))
    {
        queue_push2(jobs, SOLVER_SOLVABLE_ALL, 0);
    }
    // add mode
    for (nJob = 0; nJob < jobs->count; nJob += 2)
    {
        jobs->elements[nJob] |= mode;
        //jobs->elements[nJob] |= SOLVER_MULTIVERSION;
        if (mode == SOLVER_UPDATE && pool_isemptyupdatejob(pQuery->pSack->pPool,
                        jobs->elements[nJob], jobs->elements[nJob + 1]))
            jobs->elements[nJob] ^= SOLVER_UPDATE ^ SOLVER_INSTALL;
    }
    pSolv = solver_create(pQuery->pSack->pPool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    /* no vendor locking */
    // solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_VENDORCHANGE, 1);
    /* don't erase packages that are no longer in repo during distupgrade */
    // solver_set_flag(pSolv, SOLVER_FLAG_KEEP_ORPHANS, 1);
    /* no arch change for forcebest */
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    /* support package splits via obsoletes */
    // solver_set_flag(pSolv, SOLVER_FLAG_YUM_OBSOLETES, 1);

    solver_set_flag(pSolv, SOLVER_FLAG_SPLITPROVIDES, 1);
    if (mainMode == MODE_ERASE)
    {
        solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_UNINSTALL, 1);  /* don't nag */
    }
    // solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    //solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_DOWNGRADE, 1);
    for (;;)
    {
        if (!solver_solve(pSolv, jobs))
            break;
        nProblemCount = solver_problem_count(pSolv);
        printf("Found %d problems:\n", nProblemCount);
        for (problem = 1; problem <= nProblemCount; problem++)
        {
            printf("Problem %d/%d:\n", problem, nProblemCount);
            solver_printprobleminfo(pSolv, problem);
            printf("\n");
            nSolutionCount = solver_solution_count(pSolv, problem);
            for (solution = 1; solution <= nSolutionCount; solution++)
            {
                printf("Solution %d:\n", solution);
                solver_printsolution(pSolv, problem, solution);
                printf("\n");
            }
            solver_take_solution(pSolv, problem, 1, jobs);
        }
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
    int i = 0;
    Queue tmp;
    queue_init(&tmp);
    dwError = SolvGenerateCommonJob(pQuery);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    if(pQuery->queueJob.count > 0)
    {
        for (i = 0; i < pQuery->queueJob.count ; i += 2)
        {
            queue_empty(&tmp);
            pool_job2solvables(pQuery->pSack->pPool, &tmp,
                             pQuery->queueJob.elements[i],
                             pQuery->queueJob.elements[i + 1]);
            queue_insertn(&pQuery->queueResult, pQuery->queueResult.count, tmp.count, tmp.elements);
        }
    }
    else if(pQuery->ppszPackageNames == NULL)
    {
        pool_job2solvables(pQuery->pSack->pPool, &pQuery->queueResult, SOLVER_SOLVABLE_ALL, 0);
    }

cleanup: 
    queue_free(&tmp);
    return dwError;

error:
    goto cleanup;
}

static uint32_t
SolvApplyAlterQuery(
    PSolvQuery pQuery,
    uint32_t mainMode,
    uint32_t mode
    )
{
    uint32_t dwError = 0;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;

    dwError = SolvGenerateCommonJob(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvRunSolv(pQuery, mainMode, mode, &pQuery->queueJob, &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);

    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->queueResult, pQuery->queueResult.count, pTrans->steps.count, pTrans->steps.elements);
    //transaction_print(pTrans);
    //pQuery->nNewPackages = transaction_installedresult(pTrans, &pQuery->result);
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
    char** searchStrings,
    int startIndex,
    int endIndex
    )
{
    Queue sel, q;
    Dataiterator di = {0};
    Pool* pool = NULL;
    uint32_t dwError = 0;
    int i = 0;
    if(!pQuery || !searchStrings)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pool = pQuery->pSack->pPool;
    pool_createwhatprovides(pool);
    queue_init(&sel);
    queue_init(&q);
    for(i = startIndex; i < endIndex; i++)
    {
        queue_empty(&sel);
        queue_empty(&q);
        dataiterator_init(&di, pool, 0, 0, 0, searchStrings[i], SEARCH_SUBSTRING|SEARCH_NOCASE);
        dataiterator_set_keyname(&di, SOLVABLE_NAME);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&sel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_set_keyname(&di, SOLVABLE_SUMMARY);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&sel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_set_keyname(&di, SOLVABLE_DESCRIPTION);
        dataiterator_set_search(&di, 0, 0);
        while (dataiterator_step(&di))
            queue_push2(&sel, SOLVER_SOLVABLE, di.solvid);
        dataiterator_free(&di);
        //if (repofilter.count)
        //    selection_filter(pool, &sel, &repofilter);
        selection_solvables(pool, &sel, &q);
        queue_insertn(&pQuery->queueResult, pQuery->queueResult.count, q.count, q.elements);
    }
cleanup:
    queue_free(&sel); 
    queue_free(&q);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvSplitEvr(
    PSolvSack pSack,
    const char *pEVRstring,
    char **ppEpoch,
    char **ppVersion,
    char **ppRelease)
{

    uint32_t dwError = 0;
    char *pEvr = NULL;
    int eIndex = 0;
    int rIndex = 0;
    if(!pSack || !pEVRstring || !ppEpoch || !ppVersion || !ppRelease)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pEvr = pool_alloctmpspace(pSack->pPool, strlen(pEVRstring) + 1);
    if(!pEvr)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    strcpy(pEvr, pEVRstring);

    // EVR string format: epoch : version-release
    char* it = pEvr;
    for( ; *it != '\0'; it++)
    {
        if(*it == ':')
        {
            eIndex = it - pEvr;
        }
        else if(*it == '-')
        {
            rIndex = it - pEvr;
        }
    }

    *ppVersion = pEvr;
    *ppEpoch = NULL;
    *ppRelease = NULL;
    if(eIndex != 0)
    {
        *ppEpoch = pEvr;
        *(pEvr + eIndex) = '\0';
        *ppVersion = pEvr + eIndex + 1;
    }

    if(rIndex != 0 && rIndex > eIndex)
    {
        *ppRelease = pEvr + rIndex + 1;
        *(pEvr + rIndex) = '\0';
    }
cleanup: 
    return dwError;
error:
    goto cleanup;
}
