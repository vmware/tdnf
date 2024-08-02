/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uid_t gEuid;

static TDNF_ENV gEnv = {0};

static tdnflock instance_lock;

static void TdnfExitHandler(void);
static void IsTdnfAlreadyRunning(void);

static void TdnfExitHandler(void)
{
    if (gEuid)
    {
        return;
    }

    tdnflockFree(instance_lock);
}

static void IsTdnfAlreadyRunning(void)
{
    if (gEuid)
    {
        return;
    }

    instance_lock = tdnflockNewAcquire(TDNF_INSTANCE_LOCK_FILE,
                                       "tdnf_instance");
    if (!instance_lock)
    {
        pr_err("Failed to acquire tdnf_instance lock\n");
    }
}

uint32_t TDNFInit(void)
{
    int nLocked = 0;
    uint32_t dwError = 0;

    pthread_mutex_lock(&gEnv.mutexInitialize);
    nLocked = 1;
    if(!gEnv.nInitialized)
    {
        dwError = rpmReadConfigFiles(NULL, NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        gEnv.nInitialized = 1;
    }

cleanup:
    if(nLocked)
    {
        pthread_mutex_unlock(&gEnv.mutexInitialize);
    }
    return dwError;

error:
    goto cleanup;
}

void
TDNFUninit(
    void
    )
{
    pthread_mutex_lock (&gEnv.mutexInitialize);

    if(gEnv.nInitialized)
    {
        rpmFreeRpmrc();
    }
    gEnv.nInitialized = 0;

    pthread_mutex_unlock(&gEnv.mutexInitialize);
}

//Check all available packages
uint32_t
TDNFCheckPackages(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;
    PTDNF_CMD_ARGS pArgs = NULL;
    int nCmdCountOrig = 0;
    char **ppszCmdsOrig = NULL;
    char *ppszCheckCmds[] = {"check", "*", NULL};

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pArgs = pTdnf->pArgs;
    nCmdCountOrig = pArgs->nCmdCount;
    ppszCmdsOrig = pArgs->ppszCmds;

    //We dont intend to follow through on this install command
    pArgs->nAssumeNo = 1;

    //pass all packages available to resolve with install operation
    pArgs->nCmdCount = 2;
    pArgs->ppszCmds = ppszCheckCmds;

    dwError = TDNFResolve(pTdnf, ALTER_INSTALL, &pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pArgs)
    {
        pArgs->nCmdCount = nCmdCountOrig;
        pArgs->ppszCmds = ppszCmdsOrig;
    }
    if(pSolvedPkgInfo)
    {
       TDNFFreeSolvedPackageInfo(pSolvedPkgInfo);
    }
    return dwError;

error:
    goto cleanup;
}

//All alter commands such as install/update/erase
uint32_t
TDNFAlterCommand(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    )
{
    uint32_t dwError = 0;

    if(!pTdnf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRpmExecTransaction(pTdnf, pSolvedInfo);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFAlterHistoryCommand(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    PTDNF_HISTORY_ARGS pHistoryArgs
    )
{
    uint32_t dwError = 0;
    if(!pTdnf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRpmExecHistoryTransaction(pTdnf, pSolvedInfo, pHistoryArgs);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/**
 * Use case : tdnf check --skipconflicts --skipobsoletes
 *            tdnf check --skipconflicts
 *            tdnf check --skipobsoletes
 *            tdnf check
 * Description: This will verify if "tdnf check" command
 *              is given with --skipconflicts or --skipobsoletes
 *              or with both option, then set the problem type
 *              variable accordingly.
 * Arguments:
 *     pTdnf: Handler for TDNF command
 *     pdwSkipProblem: enum value which tells which kind of problem is set
 *
 * Return:
 *         0 : if success
 *         non zero: if error occurs
 *
 */
uint32_t
TDNFGetSkipProblemOption(
    PTDNF pTdnf,
    TDNF_SKIPPROBLEM_TYPE *pdwSkipProblem
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;

    if (!pTdnf || !pTdnf->pArgs || !pdwSkipProblem)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pdwSkipProblem = SKIPPROBLEM_NONE;

    for (pSetOpt = pTdnf->pArgs->pSetOpt; pSetOpt; pSetOpt = pSetOpt->pNext)
    {
        if (!strcasecmp(pSetOpt->pszOptName, "skipconflicts"))
        {
            *pdwSkipProblem |= SKIPPROBLEM_CONFLICTS;
        }

        if (!strcasecmp(pSetOpt->pszOptName, "skipobsoletes"))
        {
            *pdwSkipProblem |= SKIPPROBLEM_OBSOLETES;
        }
    }

    if (pTdnf->pArgs->nSkipBroken)
    {
        *pdwSkipProblem |= SKIPPROBLEM_BROKEN;
    }

cleanup:
    return dwError;

error:
    if (pdwSkipProblem)
    {
       *pdwSkipProblem = SKIPPROBLEM_NONE;
    }
    goto cleanup;
}

//check a local rpm folder for dependency issues.
uint32_t
TDNFCheckLocalPackages(
    PTDNF pTdnf,
    const char* pszLocalPath
    )
{
    uint32_t dwError = 0;
    Repo *pCmdlineRepo = 0;
    Queue queueJobs = {0};
    Solver *pSolv = NULL;
    uint32_t count = 0;
    Pool *pCmdLinePool = NULL;
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem = SKIPPROBLEM_NONE;
    Solvable *s = NULL;
    Id p;

    if(!pTdnf || !pTdnf->pSack || !pTdnf->pSack->pPool || !pszLocalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&queueJobs);

    pr_info("Checking all packages from: %s\n", pszLocalPath);

    dwError = SolvCreatePool(&pCmdLinePool);
    BAIL_ON_TDNF_ERROR(dwError);

    pool_set_rootdir(pCmdLinePool, pTdnf->pArgs->pszInstallRoot);

    pCmdlineRepo = repo_create(pCmdLinePool, CMDLINE_REPO_NAME);
    if(!pCmdlineRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvReadRpmsFromDirectory(pCmdlineRepo, pszLocalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    FOR_REPO_SOLVABLES(pCmdlineRepo, p, s) {
        queue_push2(&queueJobs, SOLVER_SOLVABLE|SOLVER_INSTALL, p);
        count++;
    }
    pr_info("Found %u packages\n", count);

    pSolv = solver_create(pCmdLinePool);
    if(pSolv == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_UNINSTALL, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_ALLOW_VENDORCHANGE, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_KEEP_ORPHANS, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_BEST_OBEY_POLICY, 1);
    solver_set_flag(pSolv, SOLVER_FLAG_YUM_OBSOLETES, 1);

    if (solver_solve(pSolv, &queueJobs))
    {
        dwError = TDNFGetSkipProblemOption(pTdnf, &dwSkipProblem);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvReportProblems(pTdnf->pSack, pSolv, dwSkipProblem);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pCmdLinePool)
    {
        pool_free(pCmdLinePool);
    }
    if(pSolv)
    {
        solver_free(pSolv);
    }
    queue_free(&queueJobs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCheckUpdates(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    dwError = TDNFList(
                  pTdnf,
                  SCOPE_UPGRADES,
                  ppszPackageNameSpecs,
                  ppPkgInfo,
                  pdwCount);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    return dwError;
}


//Clean cache data
uint32_t
TDNFClean(
    PTDNF pTdnf,
    uint32_t nCleanType
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nCleanType == CLEANTYPE_PLUGINS)
    {
        dwError = ERROR_TDNF_CLEAN_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
    {
        if (strcmp(pRepo->pszId, CMDLINE_REPO_NAME) == 0)
        {
            continue;
        }
        pr_info("cleaning %s:", pRepo->pszId);
        if (nCleanType & CLEANTYPE_METADATA)
        {
            pr_info(" metadata");
            dwError = TDNFRepoRemoveCache(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (nCleanType & CLEANTYPE_DBCACHE)
        {
            pr_info(" dbcache");
            dwError = TDNFRemoveSolvCache(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (nCleanType & CLEANTYPE_PACKAGES)
        {
            pr_info(" packages");
            dwError = TDNFRemoveRpmCache(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (nCleanType & CLEANTYPE_KEYS)
        {
            pr_info(" keys");
            dwError = TDNFRemoveKeysCache(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (nCleanType & CLEANTYPE_EXPIRE_CACHE)
        {
            pr_info(" expire-cache");
            dwError = TDNFRemoveLastRefreshMarker(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
            dwError = TDNFRemoveMirrorList(pTdnf, pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        /* remove the top level repo cache dir if it's not empty */
        dwError = TDNFRepoRemoveCacheDir(pTdnf, pRepo);
        if (dwError == ERROR_TDNF_SYSTEM_BASE + ENOTEMPTY)
        {
            /* if we did a 'clean all' the directory should be empty now. If
               not we either missed something or someone other than us
               put a file there, so warn about it, but don't bail out.
               If we did clean just one part it's not expected to be empty
               unless the other parts were already cleaned.
            */
            if (nCleanType == CLEANTYPE_ALL)
            {
                pr_err("Cache directory for %s not removed because it's not empty.\n", pRepo->pszId);
            }
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        pr_info("\n");
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

//Show count of installed
//not a tdnf command just confidence check
//equivalent to rpm -qa | wc -l
uint32_t
TDNFCountCommand(
    PTDNF pTdnf,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;

    if(!pTdnf || !pTdnf->pSack || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCountPackages(pTdnf->pSack, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    *pdwCount = dwCount;
cleanup:
    return dwError;
error:
    if(pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}

//Lists info on each installed package
//Returns a sum of installed size
uint32_t
TDNFInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    return TDNFListInternal(pTdnf, nScope,
                    ppszPackageNameSpecs,
                    ppPkgInfo, pdwCount,
                    DETAIL_INFO
                    );
}

uint32_t
TDNFList(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    )
{
    return TDNFListInternal(pTdnf, nScope,
                    ppszPackageNameSpecs,
                    ppPkgInfo, pdwCount,
                    DETAIL_LIST);
}

uint32_t
TDNFListInternal(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount,
    TDNF_PKG_DETAIL nDetail
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack || !ppszPackageNameSpecs ||
       !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    if (dwError == ERROR_TDNF_NO_MATCH && !*ppszPackageNameSpecs)
    {
        dwError = 0;
    }
    else if (dwError == 0)
    {
        dwError = TDNFPopulatePkgInfoArray(
                      pTdnf->pSack,
                      pPkgList,
                      nDetail,
                      &pPkgInfo,
                      &dwCount);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
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
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    goto cleanup;
}

//initialize tdnf and return an opaque handle
//to be used in subsequent calls.
uint32_t
TDNFOpenHandle(
    PTDNF_CMD_ARGS pArgs,
    PTDNF* ppTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF pTdnf = NULL;
    PSolvSack pSack = NULL;
    char *pszCacheDir = NULL;
    char *pszRepoDir = NULL;
    char *pszConfFile = NULL;
    char *pszConfFileInstallRoot = NULL;
    int nHasOptReposDir = 0;
    int nHasOptCacheDir = 0;
    PTDNF_CMD_OPT pOpt = NULL;

    if(!pArgs || !ppTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    gEuid = geteuid();

    IsTdnfAlreadyRunning();

    GlobalSetQuiet(pArgs->nQuiet);
    GlobalSetJson(pArgs->nJsonOutput);

    dwError = TDNFAllocateMemory(1, sizeof(TDNF), (void**)&pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pArgs = pArgs;

    /* if using --installroot, we prefer the tdnf.conf from the
    installroot unless a tdnf.conf location is explicitely set */
    if(IsNullOrEmptyString(pArgs->pszConfFile) &&
       !IsNullOrEmptyString(pArgs->pszInstallRoot) &&
       strcmp(pArgs->pszInstallRoot, "/"))
    {
        /* no conf file explicitely set in args,
        but using --installroot */

        int nExists = 0;

        /* prepend installroot to tdnf.conf location */
        dwError = TDNFJoinPath(&pszConfFileInstallRoot,
                            pArgs->pszInstallRoot,
                            TDNF_CONF_FILE,
                            NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFIsFileOrSymlink(pszConfFileInstallRoot, &nExists);
        BAIL_ON_TDNF_ERROR(dwError);

        /* if we find tdnf.conf inside the install root use it,
        otherwise use tdnf.conf from the host */
        dwError = TDNFAllocateString(
                   nExists ? pszConfFileInstallRoot : TDNF_CONF_FILE,
                   &pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateString(
                   pArgs->pszConfFile ?
                         pArgs->pszConfFile : TDNF_CONF_FILE,
                   &pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFReadConfig(
                  pTdnf,
                  pszConfFile,
                  TDNF_CONF_GROUP);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFConfigExpandVars(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    GlobalSetDnfCheckUpdateCompat(pTdnf->pConf->nCheckUpdateCompat);

    dwError = TDNFHasOpt(pTdnf->pArgs, TDNF_CONF_KEY_REPOSDIR, &nHasOptReposDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFHasOpt(pTdnf->pArgs, TDNF_CONF_KEY_CACHEDIR, &nHasOptCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!IsNullOrEmptyString(pTdnf->pArgs->pszInstallRoot) &&
        strcmp(pTdnf->pArgs->pszInstallRoot, "/"))
    {
        int nIsDir = 0;

        if(!nHasOptCacheDir)
        {
            dwError = TDNFJoinPath(&pszCacheDir,
                                   pTdnf->pArgs->pszInstallRoot,
                                   pTdnf->pConf->pszCacheDir,
                                   NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            TDNF_SAFE_FREE_MEMORY(pTdnf->pConf->pszCacheDir);
            pTdnf->pConf->pszCacheDir = pszCacheDir;
            pszCacheDir = NULL;
        }

        if (!nHasOptReposDir)
        {
            dwError = TDNFJoinPath(&pszRepoDir,
                                   pTdnf->pArgs->pszInstallRoot,
                                   pTdnf->pConf->pszRepoDir,
                                   NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFIsDir(pszRepoDir, &nIsDir);
            if (dwError == ERROR_TDNF_FILE_NOT_FOUND)
            {
                nIsDir = 0;
                dwError = 0;
            }
            BAIL_ON_TDNF_ERROR(dwError);
            if (nIsDir)
            {
                TDNF_SAFE_FREE_MEMORY(pTdnf->pConf->pszRepoDir);
                pTdnf->pConf->pszRepoDir = pszRepoDir;
                pszRepoDir = NULL;
            }
        }
    }

    /* cachedir and reoposdir when set with setopt are always relative to host */
    if (nHasOptReposDir)
    {
        TDNF_SAFE_FREE_MEMORY(pTdnf->pConf->pszRepoDir);
        dwError = TDNFGetCmdOptValue(pTdnf->pArgs, TDNF_CONF_KEY_REPOSDIR, &pTdnf->pConf->pszRepoDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nHasOptCacheDir)
    {
        TDNF_SAFE_FREE_MEMORY(pTdnf->pConf->pszCacheDir);
        dwError = TDNFGetCmdOptValue(pTdnf->pArgs, TDNF_CONF_KEY_CACHEDIR, &pTdnf->pConf->pszCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pOpt = pTdnf->pArgs->pSetOpt; pOpt; pOpt = pOpt->pNext)
    {
        /* set macros from command line */
        if (strcmp(pOpt->pszOptName, "rpmdefine") == 0)
        {
            rpmDefineMacro(NULL, pOpt->pszOptValue, 0);
        }
        /* ultimately all --setopt= settings should override config values,
           but for now make exceptions */
        else if (strcmp(pOpt->pszOptName, TDNF_CONF_KEY_KEEP_CACHE) == 0)
        {
            pTdnf->pConf->nKeepCache = isTrue(pOpt->pszOptValue);
        }
    }

    dwError = TDNFLoadPlugins(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvInitSack(
                  &pSack,
                  pTdnf->pConf->pszCacheDir,
                  pTdnf->pArgs->pszInstallRoot);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!pArgs->nAllDeps)
    {
        dwError = SolvReadInstalledRpms(pSack->pPool->installed, pszCacheDir);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = TDNFLoadRepoData(
                  pTdnf,
                  &pTdnf->pRepos);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoListFinalize(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFInitCmdLineRepo(pTdnf, pSack);
    BAIL_ON_TDNF_ERROR(dwError);

    pool_addfileprovides(pSack->pPool);
    pool_createwhatprovides(pSack->pPool);

    pTdnf->pSack = pSack;
    *ppTdnf = pTdnf;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoDir);
    TDNF_SAFE_FREE_MEMORY(pszConfFile);
    TDNF_SAFE_FREE_MEMORY(pszConfFileInstallRoot);
    return dwError;

error:
    if(pTdnf)
    {
        TDNFCloseHandle(pTdnf);
    }
    if(ppTdnf)
    {
        *ppTdnf = NULL;
    }
    if(pSack)
    {
        SolvFreeSack(pSack);
    }
    goto cleanup;
}

uint32_t
TDNFAddCmdLinePackages(
    PTDNF pTdnf,
    Queue *pQueueGoal
)
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    PSolvSack pSack;
    int nIsFile;
    int nIsRemote;
    int nCmdIndex;
    char *pszPkgName;
    char *pszCopyOfPkgName = NULL;
    char* pszRPMPath = NULL;
    Id id;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCmdArgs = pTdnf->pArgs;
    pSack = pTdnf->pSack;

    for(nCmdIndex = 1; nCmdIndex < pCmdArgs->nCmdCount; ++nCmdIndex)
    {
        /* Add packages with URLs or filenames on the command line
         * to our virtual @cmdline repo */
        pszPkgName = pCmdArgs->ppszCmds[nCmdIndex];

        /* check if it's a package file or URL */
        if (fnmatch("*.rpm", pszPkgName, 0) != 0) {
            continue;
        }

        if (fnmatch("*.src.rpm", pszPkgName, 0) == 0) {
            if (!pCmdArgs->nSource && !pCmdArgs->nBuildDeps) {
                pr_err("package '%s' appears to be a source rpm - use --source to install, or --builddeps to install its build depenfdencies\n", pszPkgName);
                dwError = ERROR_TDNF_INVALID_PARAMETER;
            }
        } else {
            if (pCmdArgs->nSource || pCmdArgs->nBuildDeps) {
                pr_err("package '%s' appears not to be a source rpm but --source or --builddeps was used\n", pszPkgName);
                dwError = ERROR_TDNF_INVALID_PARAMETER;
            }
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFIsFileOrSymlink(pszPkgName, &nIsFile);
        BAIL_ON_TDNF_ERROR(dwError);

        /* if it's a file it's to be installed
         * directly as a file. No need to download. */
        if (nIsFile)
        {
            pszRPMPath = realpath(pszPkgName, NULL);
            if (pszRPMPath == NULL)
            {
                dwError = ERROR_TDNF_SYSTEM_BASE + errno;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            dwError = TDNFUriIsRemote(pszPkgName, &nIsRemote);
            if (dwError == ERROR_TDNF_URL_INVALID)
            {
                /* not a URL => normal pkg name, nothing to do here */
                dwError = 0;
                continue;
            }
            BAIL_ON_TDNF_ERROR(dwError);
            if (!nIsRemote)
            {
                /* non-remote URL, like "file:///", no need to download */
                dwError = TDNFPathFromUri(pszPkgName, &pszRPMPath);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                PTDNF_REPO_DATA pRepo = NULL;

                /* remote URL, we need to download */
                dwError = TDNFAllocateString(pszPkgName,
                                             &pszCopyOfPkgName);
                BAIL_ON_TDNF_ERROR(dwError);

                dwError = TDNFFindRepoById(pTdnf, CMDLINE_REPO_NAME, &pRepo);
                BAIL_ON_TDNF_ERROR(dwError);

                dwError = TDNFDownloadPackageToCache(
                              pTdnf,
                              pszPkgName,
                              basename(pszCopyOfPkgName),
                              pRepo,
                              &pszRPMPath
                          );
                BAIL_ON_TDNF_ERROR(dwError);

                TDNF_SAFE_FREE_MEMORY(pszCopyOfPkgName);
           }
        }
        id = repo_add_rpm(pTdnf->pSolvCmdLineRepo, pszRPMPath,
            REPO_REUSE_REPODATA|REPO_NO_INTERNALIZE|RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM);
        if (!id)
        {
            /* TODO: get more detailed error */
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        queue_push(pQueueGoal, id);
    }

    pool_addfileprovides(pSack->pPool);
    pool_createwhatprovides(pSack->pPool);
    repo_internalize(pTdnf->pSolvCmdLineRepo);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRPMPath);
    TDNF_SAFE_FREE_MEMORY(pszCopyOfPkgName);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFProvides(
    PTDNF pTdnf,
    const char* pszSpec,
    PTDNF_PKG_INFO* ppPkgInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack || IsNullOrEmptyString(pszSpec) ||
       !ppPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszSpec);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyProvidesQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPopulatePkgInfos(pTdnf->pSack, pPkgList, &pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    return dwError;
error:
    if(ppPkgInfo)
    {
      *ppPkgInfo = NULL;
    }
    TDNFFreePackageInfo(pPkgInfo);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = ERROR_TDNF_NO_DATA;
    }
    goto cleanup;
}

uint32_t
TDNFRepoList(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA* ppReposAll
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pReposAll = NULL;
    PTDNF_REPO_DATA pRepoTemp = NULL;
    PTDNF_REPO_DATA pRepoCurrent = NULL;

    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || !pTdnf->pRepos || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        int nAdd = 0;
        if(nFilter == REPOLISTFILTER_ALL)
        {
            nAdd = 1;
        }
        else if(nFilter == REPOLISTFILTER_ENABLED && pRepos->nEnabled)
        {
            nAdd = 1;
        }
        else if(nFilter == REPOLISTFILTER_DISABLED && !pRepos->nEnabled)
        {
            nAdd = 1;
        }
        if(nAdd)
        {
            dwError = TDNFCloneRepo(pRepos, &pRepoTemp);
            BAIL_ON_TDNF_ERROR(dwError);

            if(!pReposAll)
            {
                pReposAll = pRepoTemp;
                pRepoCurrent = pReposAll;
            }
            else
            {
                pRepoCurrent->pNext = pRepoTemp;
                pRepoCurrent = pRepoCurrent->pNext;
            }
            pRepoTemp = NULL;
        }

        pRepos = pRepos->pNext;
    }

    *ppReposAll = pReposAll;

cleanup:
    return dwError;

error:
    if(ppReposAll)
    {
        *ppReposAll = NULL;
    }
    if(pReposAll)
    {
        TDNFFreeRepos(pReposAll);
    }
    goto cleanup;
}

static int
_rm_rpms(
    const char *pszFilePath,
    const struct stat *sbuf,
    int type,
    struct FTW *ftwb
    )
{
    uint32_t dwError = 0;
    char *pszKeepFile = NULL;
    struct stat statKeep = {0};

    UNUSED(sbuf);
    UNUSED(type);
    UNUSED(ftwb);

    if (strcmp(&pszFilePath[strlen(pszFilePath)-4], ".rpm") == 0)
    {
        dwError = TDNFAllocateStringPrintf(&pszKeepFile, "%s.reposync-keep", pszFilePath);
        BAIL_ON_TDNF_ERROR(dwError);

        if (stat(pszKeepFile, &statKeep))
        {
            if (errno == ENOENT)
            {
                pr_info("deleting %s\n", pszFilePath);
                if(remove(pszFilePath) < 0)
                {
                    pr_crit("unable to remove %s: %s\n", pszFilePath, strerror(errno));
                }
            }
            else
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
        }
        else
        {
            /* marker file can be removed now */
            /* coverity[toctou] */
            if(remove(pszKeepFile) < 0)
            {
                pr_crit("unable to remove %s: %s\n", pszKeepFile, strerror(errno));
            }
        }
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszKeepFile);
    return (int)dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRepoSync(
    PTDNF pTdnf,
    PTDNF_REPOSYNC_ARGS pReposyncArgs
    )
{
    uint32_t dwError = 0;
    int ret;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_REPO_DATA pRepo = NULL;
    char *pszRepoDir = NULL;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;
    char *pszRootPath = NULL;
    char *pszUrl = NULL;
    char *pszDir = NULL;
    char *pszFilePath = NULL;
    char *pszKeepFile = NULL;
    uint32_t dwCount = 0;
    uint32_t dwRepoCount = 0;
    TDNFRPMTS ts = {0};

    if(!pTdnf || !pTdnf->pSack || !pReposyncArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* count enabled repos */
    for (pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
    {
        if ((strcmp(pRepo->pszName, CMDLINE_REPO_NAME) == 0) ||
            (!pRepo->nEnabled))
        {
            continue;
        }
        dwRepoCount++;
    }

    if (dwRepoCount > 1 && pReposyncArgs->nNoRepoPath)
    {
        pr_crit("cannot use norepopath with multiple repos\n");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pReposyncArgs->nDelete && pReposyncArgs->nNoRepoPath)
    {
        /* prevent accidental deletion of packages */
        pr_crit("cannot use the delete option with norepopath\n");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pReposyncArgs->nSourceOnly && pReposyncArgs->ppszArchs)
    {
        pr_crit("cannot use the source option with arch\n");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    /* generate list of packages, result will be
       in pPkgInfos */
    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyScopeFilter(pQuery, SCOPE_ALL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPopulatePkgInfoForRepoSync(pTdnf->pSack, pPkgList, &pPkgInfos);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pReposyncArgs->nGPGCheck)
    {
        ts.pTS = rpmtsCreate();
        if(!ts.pTS)
        {
            dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    if (pReposyncArgs->pszDownloadPath == NULL)
    {
        pszRootPath = getcwd(NULL, 0);
        if (!pszRootPath)
        {
            BAIL_ON_TDNF_SYSTEM_ERROR(errno);
        }
    }
    else
    {
        dwError = TDNFNormalizePath(pReposyncArgs->pszDownloadPath, &pszRootPath);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pReposyncArgs->nNewestOnly)
    {
        TDNFPkgInfoFilterNewest(pTdnf->pSack, pPkgInfos);
    }

    /* iterate through all packages */
    for (pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        dwCount++;
        if (strcmp(pPkgInfo->pszRepoName, SYSTEM_REPO_NAME) == 0)
        {
            continue;
        }
        if (pReposyncArgs->ppszArchs)
        {
            int result = 0;
            TDNFStringMatchesOneOf(pPkgInfo->pszArch, pReposyncArgs->ppszArchs, &result);
            if (result == 0)
            {
                continue;
            }
        }
        else if (pReposyncArgs->nSourceOnly)
        {
            if (strcmp(pPkgInfo->pszArch, "src") != 0)
            {
                continue;
            }
        }

        if (!pReposyncArgs->nPrintUrlsOnly)
        {
            PTDNF_REPO_DATA pPkgRepo = NULL;

            if (!pReposyncArgs->nNoRepoPath)
            {
                dwError = TDNFJoinPath(&pszDir,
                                       pszRootPath && strcmp(pszRootPath, "/") ? pszRootPath : "",
                                       pPkgInfo->pszRepoName,
                                       NULL);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                dwError = TDNFAllocateString(pszRootPath, &pszDir);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFUtilsMakeDir(pszDir);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFFindRepoById(pTdnf, pPkgInfo->pszRepoName, &pPkgRepo);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFDownloadPackageToTree(pTdnf,
                            pPkgInfo->pszLocation, pPkgInfo->pszName,
                            pPkgRepo, pszDir,
                            &pszFilePath);
            BAIL_ON_TDNF_ERROR(dwError);

            /* if gpgcheck option is given, check for a valid signature. If that fails,
               delete the package */
            if (pReposyncArgs->nGPGCheck)
            {
                dwError = TDNFGPGCheckPackage(&ts, pTdnf, pPkgRepo, pszFilePath, NULL);
                if (dwError != RPMRC_NOTTRUSTED && dwError != RPMRC_NOKEY)
                {
                    BAIL_ON_TDNF_ERROR(dwError);
                }
                else if (dwError)
                {
                    pr_crit("checking package %s failed: %d, deleting\n", pszFilePath, dwError);
                    if(remove(pszFilePath) < 0)
                    {
                        pr_crit("unable to remove %s: %s\n", pszFilePath, strerror(errno));
                    }
                }
            }

            /* dwError==0 means TDNFGPGCheckPackage() succeeded or wasn't called */
            if (pReposyncArgs->nDelete && dwError == 0)
            {
                /* if "delete" option is given, create a marker file to protect
                   what we just downloaded. Later all *.rpm files that do not
                   have a marker file will be deleted */
                dwError = TDNFAllocateStringPrintf(&pszKeepFile, "%s.reposync-keep", pszFilePath);
                BAIL_ON_TDNF_ERROR(dwError);

                dwError = TDNFTouchFile(pszKeepFile);
                BAIL_ON_TDNF_ERROR(dwError);
                TDNF_SAFE_FREE_MEMORY(pszKeepFile);
            }
            dwError = 0;

            TDNF_SAFE_FREE_MEMORY(pszDir);
            TDNF_SAFE_FREE_MEMORY(pszFilePath);
        }
        else
        {
            /* print URLs only */
            dwError = TDNFCreatePackageUrl(pTdnf,
                                           pPkgInfo->pszRepoName,
                                           pPkgInfo->pszLocation,
                                           &pszUrl);
            BAIL_ON_TDNF_ERROR(dwError);

            pr_info("%s\n", pszUrl);

            TDNF_SAFE_FREE_MEMORY(pszUrl);
        }
    }

    if (pReposyncArgs->nDelete)
    {
        /* go through all packages in the destination directory,
           delete those that were not just downloaded as indicated by the
           marker file */
        for (pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
        {
            if ((strcmp(pRepo->pszName, CMDLINE_REPO_NAME) == 0) ||
                (!pRepo->nEnabled))
            {
                continue;
            }

            /* no need to check nNoRepoPath since we wouldn't get here */
            dwError = TDNFJoinPath(&pszRepoDir,
                                   pszRootPath && strcmp(pszRootPath, "/") ? pszRootPath : "",
                                   pRepo->pszId,
                                   NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            ret = nftw(pszRepoDir, _rm_rpms, 10, FTW_DEPTH|FTW_PHYS);
            if (ret < 0)
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
            else
            {
                dwError = ret;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    if (pReposyncArgs->nDownloadMetadata)
    {
        for (pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
        {
            if ((strcmp(pRepo->pszName, CMDLINE_REPO_NAME) == 0) ||
                (!pRepo->nEnabled))
            {
                continue;
            }

            if (!pReposyncArgs->nNoRepoPath)
            {
                const char *pszBasePath = pReposyncArgs->pszMetaDataPath ?
                                pReposyncArgs->pszMetaDataPath : pszRootPath;

                dwError = TDNFJoinPath(&pszRepoDir,
                                       pszBasePath && strcmp(pszBasePath, "/") ? pszBasePath : "",
                                       pRepo->pszId,
                                       NULL);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                dwError = TDNFAllocateString(
                            pReposyncArgs->pszMetaDataPath ?
                                pReposyncArgs->pszMetaDataPath : pszRootPath,
                            &pszRepoDir);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFUtilsMakeDir(pszRepoDir);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFDownloadMetadata(pTdnf, pRepo, pszRepoDir,
                                           pReposyncArgs->nPrintUrlsOnly);
            BAIL_ON_TDNF_ERROR(dwError);

            TDNF_SAFE_FREE_MEMORY(pszRepoDir);
        }
    }

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    if(ts.pTS)
    {
        rpmtsCloseDB(ts.pTS);
        rpmtsFree(ts.pTS);
    }
    TDNF_SAFE_FREE_MEMORY(pszDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoDir);
    TDNF_SAFE_FREE_MEMORY(pszRootPath);
    TDNF_SAFE_FREE_MEMORY(pszKeepFile);
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNFFreePackageInfoArray(pPkgInfos, dwCount);
    return dwError;
error:
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = ERROR_TDNF_NO_DATA;
    }
    goto cleanup;
}

uint32_t
TDNFRepoQuery(
    PTDNF pTdnf,
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;
    int nDetail;
    int depKey = 0;
    uint32_t dwCount = 0;
    uint32_t dwPkgIndex = 0;
    TDNF_SCOPE nScope = SCOPE_ALL;
    struct history_ctx *pHistoryCtx = NULL;

    if(!pTdnf || !pTdnf->pSack || !pRepoqueryArgs ||
       !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* check if args make sense. extra packages are installed by definition */
    if (pRepoqueryArgs->nExtras &&
        (pRepoqueryArgs->nInstalled || pRepoqueryArgs->nAvailable ||
         pRepoqueryArgs->nDuplicates))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* duplicate packages are also installed by definition */
    if (pRepoqueryArgs->nDuplicates &&
        (pRepoqueryArgs->nInstalled || pRepoqueryArgs->nAvailable))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* userinstalled implies installed */
    if (pRepoqueryArgs->nUserInstalled)
    {
        pRepoqueryArgs->nInstalled = 1;
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    /* handle select options */

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pRepoqueryArgs->nExtras)
    {
        if (!pRepoqueryArgs->nInstalled || pRepoqueryArgs->nAvailable)
        {
            nScope = SCOPE_AVAILABLE;
        }
        else if (pRepoqueryArgs->nInstalled || pRepoqueryArgs->nDuplicates)
        {
            nScope = SCOPE_INSTALLED;
        }
        else if(pRepoqueryArgs->nUpgrades)
        {
            nScope = SCOPE_UPGRADES;
        }
    }
    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    /* filter for package(s) given as arguments, if any */
    if (pRepoqueryArgs->pszSpec)
    {
        dwError = SolvApplySinglePackageFilter(pQuery, pRepoqueryArgs->pszSpec);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* run all the filters */
    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    /* filter for arch(es) */
    if (pRepoqueryArgs->ppszArchs)
    {
        dwError = SolvApplyArchFilter(pQuery, pRepoqueryArgs->ppszArchs);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* filter for extras or duplicates */
    if (pRepoqueryArgs->nExtras)
    {
        dwError = SolvApplyExtrasFilter(pQuery);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (pRepoqueryArgs->nDuplicates)
    {
        dwError = SolvApplyDuplicatesFilter(pQuery);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pRepoqueryArgs->nUserInstalled)
    {
        dwError = TDNFGetHistoryCtx(pTdnf, &pHistoryCtx, 0);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvApplyUserInstalledFilter(pQuery, pHistoryCtx);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* filter for package(s) that depend on give package(s) */
    if (pRepoqueryArgs->pppszWhatKeys)
    {
        REPOQUERY_WHAT_KEY whatKey;

        for (whatKey = 0; whatKey < REPOQUERY_WHAT_KEY_COUNT; whatKey++)
        {
            if (pRepoqueryArgs->pppszWhatKeys[whatKey])
            {
                dwError = SolvApplyDepsFilter(pQuery,
                            pRepoqueryArgs->pppszWhatKeys[whatKey],
                            whatKey);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    /* filter for package(s) that provide a given file */
    if (pRepoqueryArgs->pszFile)
    {
        dwError = SolvApplyFileProvidesFilter(pQuery, pRepoqueryArgs->pszFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* get results in list */
    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pRepoqueryArgs->pszQueryFormat)
    {
        dwError = TDNFPopulatePkgInfoQueryFormat(pTdnf->pSack, pPkgList, &pPkgInfo, &dwCount);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        /* handle query options */

        /* TDNFPopulatePkgInfoArray fills in details */
        nDetail = pRepoqueryArgs->nChangeLogs ? DETAIL_CHANGELOG :
                  pRepoqueryArgs->nSource ? DETAIL_SOURCEPKG :
                  DETAIL_LIST;
        dwError = TDNFPopulatePkgInfoArray(pTdnf->pSack, pPkgList, nDetail,
                                       &pPkgInfo, &dwCount);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* fill in file list or dependencies */
    if (pRepoqueryArgs->nList)
    {
        dwError = TDNFPopulatePkgInfoArrayFileList(
                pTdnf->pSack,
                pPkgList,
                pPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        for (dwPkgIndex =0; dwPkgIndex < dwCount; dwPkgIndex++)
        {
            dwError = TDNFAllocateMemory(
                    REPOQUERY_DEP_KEY_COUNT,
                    sizeof(char **),
                    (void **) &(&pPkgInfo[dwPkgIndex])->pppszDependencies);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        for (depKey = 0; depKey < REPOQUERY_DEP_KEY_COUNT; depKey++)
        {
            if ( pRepoqueryArgs->depKeySet & (1 << depKey))
            {
                dwError = TDNFPopulatePkgInfoArrayDependencies(
                        pTdnf->pSack,
                        pPkgList,
                        depKey,
                        pPkgInfo);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    if (pHistoryCtx)
    {
        destroy_history_ctx(pHistoryCtx);
    }
    return dwError;
error:
    TDNFFreePackageInfo(pPkgInfo);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = ERROR_TDNF_NO_DATA;
    }
    goto cleanup;
}

//Resolve alter command before presenting
//the goal steps to user for approval
uint32_t
TDNFResolve(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO* ppSolvedPkgInfo
    )
{
    uint32_t dwError = 0;
    Queue queueGoal = {0};
    char** ppszPkgsNotResolved = NULL;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;
    uint64_t qwAvailCacheBytes = 0;
    char **ppszPkgNames = NULL;
    char **ppszPkgFiles = NULL; /* cmd line packages */
    int i, iFiles = 0, iPkgs = 0;

    if(!pTdnf || !ppSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nAlterType == ALTER_INSTALL ||
        nAlterType == ALTER_REINSTALL ||
        nAlterType == ALTER_ERASE)
    {
        if(pTdnf->pArgs->nCmdCount <= 1)
        {
            dwError = ERROR_TDNF_PACKAGE_REQUIRED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    queue_init(&queueGoal);

    if(nAlterType == ALTER_INSTALL || nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFAddCmdLinePackages(pTdnf, &queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&ppszPkgNames);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&ppszPkgFiles);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 1; i < pTdnf->pArgs->nCmdCount; i++) {
        char *pszPkgName = pTdnf->pArgs->ppszCmds[i];
        if (fnmatch("*.rpm", pszPkgName, 0) == 0) {
            ppszPkgFiles[iFiles++] = pszPkgName;
        } else {
            ppszPkgNames[iPkgs++] = pszPkgName;
        }
    }

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&ppszPkgsNotResolved);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pTdnf->pArgs->nBuildDeps) {
        dwError = TDNFPrepareAllPackages(
                      pTdnf,
                      &nAlterType,
                      ppszPkgsNotResolved,
                      &queueGoal);
    } else {
        dwError = TDNFResolveBuildDependencies(
                        pTdnf,
                        ppszPkgNames,
                        ppszPkgsNotResolved,
                        &queueGoal);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pTdnf->pArgs->nSource && !pTdnf->pArgs->nNoDeps) {
        dwError = TDNFGoal(
                      pTdnf,
                      &queueGoal,
                      &pSolvedPkgInfo,
                      nAlterType);
    } else {
        dwError = TDNFGoalNoDeps(
                      pTdnf,
                      &queueGoal,
                      &pSolvedPkgInfo);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    pSolvedPkgInfo->nNeedAction =
        pSolvedPkgInfo->pPkgsToInstall ||
        pSolvedPkgInfo->pPkgsToUpgrade ||
        pSolvedPkgInfo->pPkgsToDowngrade ||
        pSolvedPkgInfo->pPkgsToRemove  ||
        pSolvedPkgInfo->pPkgsUnNeeded ||
        pSolvedPkgInfo->pPkgsToReinstall ||
        pSolvedPkgInfo->pPkgsObsoleted;

    pSolvedPkgInfo->nNeedDownload =
        pSolvedPkgInfo->pPkgsToInstall ||
        pSolvedPkgInfo->pPkgsToUpgrade ||
        pSolvedPkgInfo->pPkgsToDowngrade ||
        pSolvedPkgInfo->pPkgsToReinstall;

    dwError = TDNFGetAvailableCacheBytes(pTdnf->pConf, &qwAvailCacheBytes);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pSolvedPkgInfo->nNeedDownload)
    {
        dwError = TDNFCheckDownloadCacheBytes(pSolvedPkgInfo, qwAvailCacheBytes);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSolvedPkgInfo->ppszPkgsNotResolved = ppszPkgsNotResolved;
    *ppSolvedPkgInfo = pSolvedPkgInfo;

cleanup:
    /* only free the pointers */
    TDNF_SAFE_FREE_MEMORY(ppszPkgNames);
    TDNF_SAFE_FREE_MEMORY(ppszPkgFiles);

    queue_free(&queueGoal);
    return dwError;

error:
    if(ppSolvedPkgInfo)
    {
        *ppSolvedPkgInfo = NULL;
    }
    if(pSolvedPkgInfo)
    {
        TDNFFreeSolvedPackageInfo(pSolvedPkgInfo);
    }
    TDNF_SAFE_FREE_STRINGARRAY(ppszPkgsNotResolved);
    goto cleanup;
}

uint32_t
TDNFSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* punCount
    )
{
    uint32_t dwError = 0;
    int nStartArgIndex = 1;
    PSolvQuery pQuery = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvPackageList pPkgList = NULL;
    int nIndex = 0;
    uint32_t unCount  = 0;
    Id dwPkgId = 0;
    if(!pTdnf || !pCmdArgs || !ppPkgInfo || !punCount || !pTdnf->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount > 1)
    {
        if(!strncasecmp(pCmdArgs->ppszCmds[1], "all", 3))
        {
            nStartArgIndex = 2;
        }
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySearch(
                  pQuery,
                  pCmdArgs->ppszCmds,
                  nStartArgIndex,
                  pCmdArgs->nCmdCount);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pPkgList, &unCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if (unCount < 1)
    {
        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  unCount,
                  sizeof(TDNF_PKG_INFO),
                  (void**)&pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    for(nIndex = 0; (uint32_t)nIndex < unCount; nIndex++)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];

        dwError = SolvGetPackageId(pPkgList, nIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pTdnf->pSack, dwPkgId, &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgSummaryFromId(
                      pTdnf->pSack,
                      dwPkgId,
                      &pPkg->pszSummary);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppPkgInfo = pPkgInfo;
    *punCount = unCount;

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    return dwError;
error:
    if(ppPkgInfo)
    {
        *ppPkgInfo = NULL;
    }
    if(punCount)
    {
        *punCount = 0;
    }
    TDNFFreePackageInfoArray(pPkgInfo, unCount);

    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
    }

    goto cleanup;
}

//TODO: Refactor UpdateInfoSummary into one function
uint32_t
TDNFUpdateInfo(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO* ppUpdateInfo
    )
{
    uint32_t dwError = 0;
    uint32_t nCount = 0;
    int iAdv = 0;
    uint32_t dwPkgIndex = 0;
    uint32_t dwSize = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pUpdateAdvPkgList = NULL;
    Id dwAdvId = 0;
    Id dwPkgId = 0;
    uint32_t dwRebootRequired = 0;

    PTDNF_UPDATEINFO pUpdateInfos = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;

    char*  pszSeverity = NULL;
    uint32_t dwSecurity = 0;
    int nUpdates = 0;

    if(!pTdnf || !pTdnf->pSack || !pTdnf->pSack->pPool ||
       !ppUpdateInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!ppszPackageNameSpecs)
    {
        dwError = SolvFindAllInstalled(pTdnf->pSack, &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = SolvFindInstalledPkgByMultipleNames(
                      pTdnf->pSack,
                      ppszPackageNameSpecs,
                      &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetSecuritySeverityOption(
                  pTdnf,
                  &dwSecurity,
                  &pszSeverity);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetRebootRequiredOption(
                  pTdnf,
                  &dwRebootRequired);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetUpdateAdvisories(
                      pTdnf->pSack,
                      dwPkgId,
                      &pUpdateAdvPkgList);
        //Ignore no data and continue.
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
            continue;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPackageListSize(pUpdateAdvPkgList, &nCount);
        BAIL_ON_TDNF_ERROR(dwError);

        for(iAdv = 0; (uint32_t)iAdv < nCount; iAdv++)
        {
            dwError = SolvGetPackageId(pUpdateAdvPkgList, iAdv, &dwAdvId);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFPopulateUpdateInfoOfOneAdvisory(
                          pTdnf->pSack,
                          dwAdvId,
                          dwSecurity,
                          pszSeverity,
                          dwRebootRequired,
                          &pInfo);
            BAIL_ON_TDNF_ERROR(dwError);

            if(pInfo)
            {
                nUpdates++;
                pInfo->pNext = pUpdateInfos;
                pUpdateInfos = pInfo;
                pInfo = NULL;
            }
        }
        SolvFreePackageList(pUpdateAdvPkgList);
        pUpdateAdvPkgList = NULL;
    }

    if(!pUpdateInfos)
    {
        pr_info("\n%d updates.\n", nUpdates);
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppUpdateInfo = pUpdateInfos;

cleanup:
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    if(pUpdateAdvPkgList)
    {
        SolvFreePackageList(pUpdateAdvPkgList);
    }
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    return dwError;

error:
    if(ppUpdateInfo)
    {
        *ppUpdateInfo = NULL;
    }
    if(pUpdateInfos)
    {
        TDNFFreeUpdateInfo(pUpdateInfos);
    }
    if(pInfo)
    {
        TDNFFreeUpdateInfo(pInfo);
    }
    goto cleanup;
}

uint32_t
TDNFHistoryResolve(
    PTDNF pTdnf,
    PTDNF_HISTORY_ARGS pHistoryArgs,
    PTDNF_SOLVED_PKG_INFO *ppSolvedPkgInfo)
{
    uint32_t dwError = 0;
    int rc = 0;
    char **ppszPkgsNotResolved = NULL;
    Queue queueGoal = {0};
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;
    struct history_ctx *ctx = NULL;
    struct history_delta *hd = NULL;
    struct history_flags_delta *hfd = NULL;
    struct history_nevra_map *hnm = NULL;
    rpmts ts = NULL;
    Queue qInstall = {0};
    Queue qErase = {0};

    if(!pTdnf || !pHistoryArgs || !ppSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* args sanity checks */
    if (pHistoryArgs->nCommand == HISTORY_CMD_ROLLBACK)
    {
        if (pHistoryArgs->nTo <= 0)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else if (pHistoryArgs->nCommand == HISTORY_CMD_UNDO ||
             pHistoryArgs->nCommand == HISTORY_CMD_REDO)
    {
        if (pHistoryArgs->nFrom <= 1 || /* cannot undo or redo the base set */
            pHistoryArgs->nFrom > pHistoryArgs->nTo)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else if (pHistoryArgs->nCommand != HISTORY_CMD_INIT)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* no need to refresh cache when initializing db */
    if (pHistoryArgs->nCommand != HISTORY_CMD_INIT) {
        dwError = TDNFRefresh(pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ts = rpmtsCreate();
    if(!ts)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (rpmtsOpenDB(ts, O_RDONLY))
    {
        dwError = ERROR_TDNF_RPMTS_OPENDB_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmtsSetRootDir(ts, pTdnf->pArgs->pszInstallRoot))
    {
        dwError = ERROR_TDNF_RPMTS_BAD_ROOT_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetHistoryCtx(pTdnf, &ctx,
                                pHistoryArgs->nCommand != HISTORY_CMD_INIT);
    BAIL_ON_TDNF_ERROR(dwError);

    rc = history_sync(ctx, ts);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    switch (pHistoryArgs->nCommand)
    {
        case HISTORY_CMD_INIT:
            goto cleanup;
        case HISTORY_CMD_ROLLBACK:
            hd = history_get_delta(ctx, pHistoryArgs->nTo);
            hfd = history_get_flags_delta(ctx, ctx->trans_id, pHistoryArgs->nTo);
            break;
        case HISTORY_CMD_UNDO:
            hd = history_get_delta_range(ctx, pHistoryArgs->nFrom - 1, pHistoryArgs->nTo);
            hfd = history_get_flags_delta(ctx, pHistoryArgs->nTo, pHistoryArgs->nFrom - 1);
            break;
        case HISTORY_CMD_REDO:
            hd = history_get_delta_range(ctx, pHistoryArgs->nTo, pHistoryArgs->nFrom - 1);
            hfd = history_get_flags_delta(ctx, pHistoryArgs->nFrom - 1, pHistoryArgs->nTo);
            break;
        default:
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
    }

    if (hd == NULL)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (hd->added_count > 0)
    {
        dwError = TDNFAllocateMemory(
                      hd->added_count+1, /* only added pkgs, plus a NULL ptr */
                      sizeof(char*),
                      (void**)&ppszPkgsNotResolved);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hnm = history_nevra_map(ctx);
    if (hnm == NULL)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&qInstall);
    queue_init(&qErase);

    for (int i = 0; i < hd->added_count; i++)
    {
        const char *pszPkgName = history_get_nevra(hnm, hd->added_ids[i]);
        if (pszPkgName)
        {
            if (strncmp(pszPkgName, "gpg-pubkey-", 11) == 0)
                continue;

            Queue qResult = {0};
            queue_init(&qResult);

            dwError = SolvFindSolvablesByNevraStr(pTdnf->pSack->pPool,
                                                  pszPkgName, &qResult, SOLV_NEVRA_UNINSTALLED);
            BAIL_ON_TDNF_ERROR(dwError);

            if (qResult.count == 0)
            {
                dwError = TDNFAddNotResolved(ppszPkgsNotResolved, pszPkgName);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                Queue qInstalled = {0};
                queue_init(&qInstalled);

                /* find if pkg is already installed */
                /* TODO: make this more efficient by using the pool ids of the solvable
                   with SolvFindSolvablesByNevraId() */
                dwError = SolvFindSolvablesByNevraStr(pTdnf->pSack->pPool,
                                                      pszPkgName, &qInstalled, SOLV_NEVRA_INSTALLED);
                BAIL_ON_TDNF_ERROR(dwError);

                if (qInstalled.count == 0)
                {
                    /* We may have found multiples if they occur in multiple
                       repos. Take the first one. */
                    queue_push(&qInstall, qResult.elements[0]);
                }
                queue_free(&qInstalled);
            }
            queue_free(&qResult);
        }
        else
        {
            dwError = ERROR_TDNF_HISTORY_ERROR;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    for (int i = 0; i < hd->removed_count; i++)
    {
        const char *pszPkgName = history_get_nevra(hnm, hd->removed_ids[i]);
        if (pszPkgName)
        {
            if (strncmp(pszPkgName, "gpg-pubkey-", 11) == 0)
                continue;

            dwError = SolvFindSolvablesByNevraStr(pTdnf->pSack->pPool, pszPkgName, &qErase, SOLV_NEVRA_INSTALLED);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_TDNF_HISTORY_ERROR;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFHistoryGoal(
                  pTdnf,
                  &qInstall,
                  &qErase,
                  &pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pSolvedPkgInfo->nNeedAction =
        pSolvedPkgInfo->pPkgsToInstall ||
        pSolvedPkgInfo->pPkgsToUpgrade ||
        pSolvedPkgInfo->pPkgsToDowngrade ||
        pSolvedPkgInfo->pPkgsToRemove  ||
        pSolvedPkgInfo->pPkgsUnNeeded ||
        pSolvedPkgInfo->pPkgsToReinstall ||
        pSolvedPkgInfo->pPkgsObsoleted;

    pSolvedPkgInfo->nNeedDownload =
        pSolvedPkgInfo->pPkgsToInstall ||
        pSolvedPkgInfo->pPkgsToUpgrade ||
        pSolvedPkgInfo->pPkgsToDowngrade ||
        pSolvedPkgInfo->pPkgsToReinstall;

    pSolvedPkgInfo->ppszPkgsNotResolved = ppszPkgsNotResolved;

    /* if there is no action, maybe there is a flags change */
    if (!pSolvedPkgInfo->nNeedAction)
    {
        pSolvedPkgInfo->nNeedAction = (hfd->count > 0);
    }

    *ppSolvedPkgInfo = pSolvedPkgInfo;

cleanup:
    history_free_nevra_map(hnm);
    history_free_delta(hd);
    history_free_flags_delta(hfd);
    destroy_history_ctx(ctx);
    queue_free(&queueGoal);
    queue_free(&qInstall);
    queue_free(&qErase);
    if (ts) {
        rpmtsCloseDB(ts);
        rpmtsFree(ts);
    }
    return dwError;

error:
    if(pSolvedPkgInfo)
    {
        TDNFFreeSolvedPackageInfo(pSolvedPkgInfo);
    }
    if(ppszPkgsNotResolved)
    {
        TDNFFreeStringArray(ppszPkgsNotResolved);
    }
    goto cleanup;
}

uint32_t
TDNFHistoryList(
    PTDNF pTdnf,
    PTDNF_HISTORY_ARGS pHistoryArgs,
    PTDNF_HISTORY_INFO *ppHistoryInfo)
{
    uint32_t dwError = 0;
    struct history_transaction *tas = NULL;
    struct history_nevra_map *hnm = NULL;
    int count = 0;
    PTDNF_HISTORY_INFO pHistoryInfo = NULL;
    PTDNF_HISTORY_INFO_ITEM pHistoryInfoItems = NULL;
    struct history_ctx *ctx = NULL;
    int rc = 0;

    if(!pTdnf || !pHistoryArgs || !ppHistoryInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pHistoryArgs->nFrom < 0 || pHistoryArgs->nTo < 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pHistoryArgs->nFrom != 0 && pHistoryArgs->nTo != 0)
    {
        if (pHistoryArgs->nFrom > pHistoryArgs->nTo)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFGetHistoryCtx(pTdnf, &ctx, 1);
    BAIL_ON_TDNF_ERROR(dwError);

    rc = history_get_transactions(ctx, &tas, &count,
                             pHistoryArgs->nReverse,
                             pHistoryArgs->nFrom, pHistoryArgs->nTo);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  count,
                  sizeof(TDNF_HISTORY_INFO_ITEM),
                  (void**)&pHistoryInfoItems);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pHistoryArgs->nInfo)
    {
        hnm = history_nevra_map(ctx);
    }

    for(int i = 0; i < count; i++)
    {
        pHistoryInfoItems[i].nId = tas[i].id;
        pHistoryInfoItems[i].nType = tas[i].type;
        dwError = TDNFAllocateString(tas[i].cmdline ?
            tas[i].cmdline :
            "(none)",
            &pHistoryInfoItems[i].pszCmdLine);
        BAIL_ON_TDNF_ERROR(dwError);
        pHistoryInfoItems[i].timeStamp = tas[i].timestamp;
        pHistoryInfoItems[i].nAddedCount = tas[i].delta.added_count;
        pHistoryInfoItems[i].nRemovedCount = tas[i].delta.removed_count;

        if (!hnm)
            continue;

        if (tas[i].delta.added_count > 0)
        {
            dwError = TDNFAllocateMemory(tas[i].delta.added_count,
                                         sizeof(char *),
                                         (void **)&pHistoryInfoItems[i].ppszAddedPkgs);
            BAIL_ON_TDNF_ERROR(dwError);
            for (int j = 0; j < tas[i].delta.added_count; j++)
            {
                dwError = TDNFAllocateString(history_get_nevra(hnm, tas[i].delta.added_ids[j]),
                        &pHistoryInfoItems[i].ppszAddedPkgs[j]);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }

        if (tas[i].delta.removed_count > 0)
        {
            dwError = TDNFAllocateMemory(tas[i].delta.removed_count,
                                         sizeof(char *),
                                         (void **)&pHistoryInfoItems[i].ppszRemovedPkgs);
            BAIL_ON_TDNF_ERROR(dwError);
            for (int j = 0; j < tas[i].delta.removed_count; j++)
            {
                dwError = TDNFAllocateString(history_get_nevra(hnm, tas[i].delta.removed_ids[j]),
                        &pHistoryInfoItems[i].ppszRemovedPkgs[j]);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    dwError = TDNFAllocateMemory(
                  count,
                  sizeof(TDNF_HISTORY_INFO),
                  (void**)&pHistoryInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pHistoryInfo->nItemCount = count;
    pHistoryInfo->pItems = pHistoryInfoItems;
    *ppHistoryInfo = pHistoryInfo;

cleanup:
    history_free_nevra_map(hnm);
    history_free_transactions(tas, count);
    destroy_history_ctx(ctx);
    return dwError;

error:
    if (pHistoryInfoItems)
    {
        TDNFFreeHistoryInfoItems(pHistoryInfoItems, count);
    }
    goto cleanup;
}

uint32_t
TDNFMark(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    uint32_t dwValue
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;
    struct history_ctx *ctx = NULL;
    char *pszCmdLine = NULL;
    char *pszName = NULL;

    if(!pTdnf || !pTdnf->pSack || !ppszPackageNameSpecs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyScopeFilter(pQuery, SCOPE_INSTALLED);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pPkgList->queuePackages.count > 0)
    {
        dwError = TDNFJoinArrayToString(&(pTdnf->pArgs->ppszArgv[1]),
                                        " ",
                                        pTdnf->pArgs->nArgc,
                                        &pszCmdLine);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFGetHistoryCtx(pTdnf, &ctx, 1);
        BAIL_ON_TDNF_ERROR(dwError);

        history_add_transaction(ctx, pszCmdLine);
        BAIL_ON_TDNF_ERROR(dwError);

        for (int i = 0; i < pPkgList->queuePackages.count; i++)
        {
            int rc;

            dwError = SolvGetPkgNameFromId(pTdnf->pSack,
                                           pPkgList->queuePackages.elements[i],
                                           &pszName);
            BAIL_ON_TDNF_ERROR(dwError);

            pr_info("marking %s as %sinstalled\n", pszName, dwValue ? "auto" : "user");

            rc = history_set_auto_flag(ctx, pszName, dwValue);
            if (rc != 0)
            {
                dwError = ERROR_TDNF_HISTORY_ERROR;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            TDNF_SAFE_FREE_MEMORY(pszName);
        }
    }
    else
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCmdLine);
    TDNF_SAFE_FREE_MEMORY(pszName);
    destroy_history_ctx(ctx);
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    return dwError;
error:
    goto cleanup;
}

//api calls to free memory allocated by tdnfclientlib
void
TDNFCloseHandle(
    PTDNF pTdnf
    )
{
    if(pTdnf)
    {
        if(pTdnf->pRepos)
        {
            TDNFFreeReposInternal(pTdnf->pRepos);
        }
        if(pTdnf->pConf)
        {
            TDNFFreeConfig(pTdnf->pConf);
        }
        if(pTdnf->pSack)
        {
            SolvFreeSack(pTdnf->pSack);
        }
        TDNFFreePlugins(pTdnf->pPlugins);
        TDNFFreeMemory(pTdnf);
    }
    TdnfExitHandler();
}

const char*
TDNFGetVersion(
    )
{
    return PACKAGE_VERSION;
}

const char*
TDNFGetPackageName(
    )
{
    return PACKAGE_NAME;
}
