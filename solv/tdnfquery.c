#include "includes.h"

#define MODE_INSTALL     1
#define MODE_ERASE       2
#define MODE_UPDATE      3
#define MODE_DISTUPGRADE 4
#define MODE_VERIFY      5
#define MODE_PATCH       6

PSolvQuery
SolvCreateQuery(
    PSolvSack pSack)
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
    pQuery->pSack = pSack;
    queue_init(&pQuery->job);
    pQuery->pSolv = NULL;
    pQuery->pTrans = NULL;
    pQuery->packageNames = NULL;
    queue_init(&pQuery->repoFilter);
    queue_init(&pQuery->kindFilter);
    queue_init(&pQuery->archFilter);
    queue_init(&pQuery->result);
cleanup: 
    return pQuery;

error:
    goto cleanup;
}

void
SolvFreeQuery(
    PSolvQuery pQuery)
{
    char** tmpNames = NULL;
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

        queue_free(&pQuery->job);
        queue_free(&pQuery->repoFilter);
        queue_free(&pQuery->kindFilter);
        queue_free(&pQuery->archFilter);
        queue_free(&pQuery->result);
        if(pQuery->packageNames)
        {
            tmpNames = pQuery->packageNames;
            while(*tmpNames)
            {
                solv_free(*tmpNames);
                tmpNames++;
            }
            solv_free(pQuery->packageNames);
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
    pQuery->packageNames = ppCopyOfpkgNames;

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
        pQuery->packageNames = ppCopyOfpkgNames;
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
    queue_push2(&pQuery->repoFilter, SOLVER_SOLVABLE_REPO | SOLVER_SETREPO, pool->installed->repoid);

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
            queue_push2(&pQuery->repoFilter, SOLVER_SOLVABLE_REPO | SOLVER_SETREPO | SOLVER_SETVENDOR, pRepo->repoid);
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
    char** pkgNames = pQuery->packageNames;
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
            if (pQuery->repoFilter.count)
                selection_filter(pPool, &job2, &pQuery->repoFilter);
            if (pQuery->archFilter.count)
                selection_filter(pPool, &job2, &pQuery->archFilter);
            if (pQuery->kindFilter.count)
                selection_filter(pPool, &job2, &pQuery->kindFilter);
            if (!job2.count)
            {
                flags |= SELECTION_NOCASE;

                rflags = selection_make(pPool, &job2, *pkgNames, flags);
                if (pQuery->repoFilter.count)
                    selection_filter(pPool, &job2, &pQuery->repoFilter);
                if (pQuery->archFilter.count)
                    selection_filter(pPool, &job2, &pQuery->archFilter);
                if (pQuery->kindFilter.count)
                    selection_filter(pPool, &job2, &pQuery->kindFilter);
                if (job2.count)
                    printf("[ignoring case for '%s']\n", *pkgNames);
            }
            if (job2.count)
            {
                if (rflags & SELECTION_FILELIST)
                    printf("[using file list match for '%s']\n", *pkgNames);
                if (rflags & SELECTION_PROVIDES)
                    printf("[using capability match for '%s']\n", *pkgNames);
                queue_insertn(&pQuery->job, pQuery->job.count, job2.count, job2.elements);
            }
            pkgNames++;
        }
    }
    else if(pQuery->repoFilter.count || 
            pQuery->archFilter.count || 
            pQuery->kindFilter.count)
    {
        queue_empty(&job2);
        queue_push2(&job2, SOLVER_SOLVABLE_ALL, 0);
        if (pQuery->repoFilter.count)
            selection_filter(pPool, &job2, &pQuery->repoFilter);
        if (pQuery->archFilter.count)
            selection_filter(pPool, &job2, &pQuery->archFilter);
        if (pQuery->kindFilter.count)
            selection_filter(pPool, &job2, &pQuery->kindFilter);
        queue_insertn(&pQuery->job, pQuery->job.count, job2.count, job2.elements);
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
    if(pQuery->job.count > 0)
    {
        for (i = 0; i < pQuery->job.count ; i += 2)
        {
            queue_empty(&tmp);
            pool_job2solvables(pQuery->pSack->pPool, &tmp,
                             pQuery->job.elements[i],
                             pQuery->job.elements[i + 1]);
            queue_insertn(&pQuery->result, pQuery->result.count, tmp.count, tmp.elements);
        }
    }
    else if(pQuery->packageNames == NULL)
    {
        pool_job2solvables(pQuery->pSack->pPool, &pQuery->result, SOLVER_SOLVABLE_ALL, 0);
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

    dwError = SolvRunSolv(pQuery, mainMode, mode, &pQuery->job, &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);

    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->result, pQuery->result.count, pTrans->steps.count, pTrans->steps.elements);
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
SolvApplyEraseQuery(
    PSolvQuery pQuery
    )
{
    return SolvApplyAlterQuery(pQuery, MODE_ERASE, SOLVER_ERASE);
}

uint32_t
SolvApplyInstallQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    Id recent = 0;
    Queue tmp;
    Queue tmp2;
    Queue jobs;
    queue_init(&tmp);
    queue_init(&tmp2);
    queue_init(&jobs);
    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_insertn(&tmp, tmp.count, pQuery->result.count, pQuery->result.elements);
    while(tmp.count > 0 )
    {
        dwError = SolvGetLatest(pQuery->pSack, &tmp, tmp.elements[0], &recent);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push2(&jobs, SOLVER_SOLVABLE|SOLVER_INSTALL, recent);
        dwError = SolvRemovePkgWithSameName(pQuery->pSack, &tmp,
                 tmp.elements[0], &tmp2);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_empty(&tmp);
        queue_insertn(&tmp, tmp.count, tmp2.count, tmp2.elements);
        queue_empty(&tmp2);
    }

    dwError = SolvRunSolv(pQuery, MODE_INSTALL, SOLVER_INSTALL, &jobs, &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);
    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->result, pQuery->result.count, pTrans->steps.count, pTrans->steps.elements);
    transaction_print(pTrans);
cleanup:
    queue_free(&jobs);
    queue_free(&tmp);
    queue_free(&tmp2);
    if(pSolv)
        solver_free(pSolv);
    return dwError;

error:
    if(pTrans)
        transaction_free(pTrans);
    goto cleanup;
}

uint32_t
SolvApplyReinstallQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    int pkgIndex = 0;
    Queue jobs;
    queue_init(&jobs);
    for(pkgIndex = 0 ; pkgIndex < pQuery->result.count; pkgIndex++)
    {
        queue_push2(&jobs, SOLVER_SOLVABLE|SOLVER_INSTALL, pQuery->result.elements[pkgIndex]);
    }
    dwError = SolvRunSolv(pQuery, MODE_INSTALL, SOLVER_INSTALL, &jobs, &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);
    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->result, pQuery->result.count, pTrans->steps.count, pTrans->steps.elements);
    transaction_print(pTrans);
cleanup:
    queue_free(&jobs);
    if(pSolv)
        solver_free(pSolv);
    return dwError;

error:
    if(pTrans)
        transaction_free(pTrans);
    goto cleanup;
}
uint32_t
SolvApplyUpdateQuery(
    PSolvQuery pQuery
    )
{
    return SolvApplyAlterQuery(pQuery, MODE_UPDATE, SOLVER_UPDATE);
}

uint32_t
SolvApplyDowngradeQuery(
    PSolvQuery pQuery
    )
{
    uint32_t dwError = 0;
    Solver *pSolv = NULL;
    Transaction *pTrans = NULL;
    Id recent = 0;
    Queue tmp;
    Queue tmp2;
    Queue jobs;
    queue_init(&tmp);
    queue_init(&tmp2);
    queue_init(&jobs);

    queue_insertn(&tmp, tmp.count, pQuery->result.count, pQuery->result.elements);
    while(tmp.count > 0 )
    {
        dwError = SolvGetLatest(pQuery->pSack, &tmp, tmp.elements[0], &recent);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_push2(&jobs, SOLVER_SOLVABLE|SOLVER_INSTALL, recent);
        dwError = SolvRemovePkgWithSameName(pQuery->pSack, &tmp,
                 tmp.elements[0], &tmp2);
        BAIL_ON_TDNF_ERROR(dwError);
        queue_empty(&tmp);
        queue_insertn(&tmp, tmp.count, tmp2.count, tmp2.elements);
        queue_empty(&tmp2);
    }

    dwError = SolvRunSolv(pQuery, MODE_INSTALL, SOLVER_INSTALL, &jobs, &pSolv);
    BAIL_ON_TDNF_ERROR(dwError);
    pTrans = solver_create_transaction(pSolv);
    if (!pTrans->steps.count)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pQuery->pTrans = pTrans;
    queue_insertn(&pQuery->result, pQuery->result.count, pTrans->steps.count, pTrans->steps.elements);
    transaction_print(pTrans);
cleanup:
    queue_free(&jobs);
    queue_free(&tmp);
    queue_free(&tmp2);
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
        queue_insertn(&pQuery->result, pQuery->result.count, q.count, q.elements);
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
