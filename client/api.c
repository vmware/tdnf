/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : api.c
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

static TDNF_ENV gEnv = {0};

uint32_t
TDNFInit(
    )
{
    uint32_t dwError = 0;
    int nLocked = 0;

    pthread_mutex_lock (&gEnv.mutexInitialize);
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

uint32_t
TDNFIsInitialized(
    int *pnInitialized
    )
{
    uint32_t dwError = 0;
    int nInitialized = 0;
    int nLocked = 0;

    if(!pnInitialized)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pthread_mutex_lock (&gEnv.mutexInitialize);
    nLocked = 1;

    nInitialized = gEnv.nInitialized;

    *pnInitialized = nInitialized;

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

    return dwError;

error:
    goto cleanup;
}

//All alter commands such as install/update/erase
uint32_t
TDNFAlterCommand(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    )
{
    uint32_t dwError = 0;
    if(!pTdnf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFRpmExecTransaction(pTdnf, pSolvedInfo, nAlterType);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
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
    char* pszRPMPath = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    Repo *pCmdlineRepo = 0;
    Id    dwPkgAdded = 0;
    Queue queueJobs = {0};
    Solver *pSolv = NULL;
    uint32_t dwPackagesFound = 0;
    int nLen = 0;
    int nLenRpmExt = 0;
    Pool *pCmdLinePool = NULL;

    if(!pTdnf || !pTdnf->pSack || !pTdnf->pSack->pPool || !pszLocalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&queueJobs);
    pDir = opendir(pszLocalPath);
    if(pDir == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fprintf(stdout, "Checking all packages from: %s\n", pszLocalPath);

    pCmdLinePool = pool_create();
    pool_set_rootdir(pCmdLinePool, pTdnf->pArgs->pszInstallRoot);

    pCmdlineRepo = repo_create(pCmdLinePool, CMDLINE_REPO_NAME);
    if(!pCmdlineRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((pEnt = readdir (pDir)) != NULL )
    {
        nLenRpmExt = strlen(TDNF_RPM_EXT);
        nLen = strlen(pEnt->d_name);
        if (nLen <= nLenRpmExt ||
            strcmp(pEnt->d_name + nLen - nLenRpmExt, TDNF_RPM_EXT))
        {
            continue;
        }
        dwError = TDNFAllocateStringPrintf(
                      &pszRPMPath,
                      "%s/%s",
                      pszLocalPath,
                      pEnt->d_name);
        BAIL_ON_TDNF_ERROR(dwError);

        dwPkgAdded = repo_add_rpm(
                         pCmdlineRepo,
                         pszRPMPath,
                         REPO_REUSE_REPODATA|REPO_NO_INTERNALIZE);
        if(!dwPkgAdded)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        queue_push2(&queueJobs, SOLVER_SOLVABLE|SOLVER_INSTALL, dwPkgAdded);
        dwPackagesFound++;
        TDNF_SAFE_FREE_MEMORY(pszRPMPath);
        pszRPMPath = NULL;
    }
    repo_internalize(pCmdlineRepo);
    fprintf(stdout, "Found %d packages\n", dwPackagesFound);

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

    if (solver_solve(pSolv, &queueJobs) != 0)
    {
        dwError = SolvReportProblems(pSolv);
        BAIL_ON_TDNF_ERROR(dwError);

        //Fail the check
        dwError = ERROR_TDNF_SOLV_FAILED;
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
    if(pDir)
    {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszRPMPath);
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
    TDNF_CLEANTYPE nCleanType,
    PTDNF_CLEAN_INFO* ppCleanInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_CLEAN_INFO pCleanInfo = NULL;
    char** ppszReposUsed = NULL;

    if(!pTdnf || !ppCleanInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Rest of the clean options will be
    //supported in the next release
    if(nCleanType != CLEANTYPE_ALL)
    {
        dwError = ERROR_TDNF_CLEAN_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CLEAN_INFO),
                  (void**)&pCleanInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCopyEnabledRepos(pTdnf->pRepos, &pCleanInfo->ppszReposUsed);
    BAIL_ON_TDNF_ERROR(dwError);

    ppszReposUsed = pCleanInfo->ppszReposUsed;
    if(nCleanType == CLEANTYPE_ALL)
    {
        while(*ppszReposUsed)
        {
            dwError = TDNFRepoRemoveCache(pTdnf,*ppszReposUsed);
            BAIL_ON_TDNF_ERROR(dwError);

            ++ppszReposUsed;
        }
    }

    pCleanInfo->nCleanAll = (nCleanType == CLEANTYPE_ALL);

    *ppCleanInfo = pCleanInfo;
cleanup:
    return dwError;

error:
    if(ppCleanInfo)
    {
        *ppCleanInfo = NULL;
    }
    if(pCleanInfo)
    {
        TDNFFreeCleanInfo(pCleanInfo);
    }
    goto cleanup;
}

//Show count of installed
//not a dnf command just sanity check
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
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PSolvQuery pQuery = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pTdnf || !pTdnf->pSack ||!pdwCount || !ppPkgInfo || 
       !ppszPackageNameSpecs || !pTdnf->pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPopulatePkgInfoArray(
                  pTdnf->pSack,
                  pPkgList,
                  DETAIL_INFO,
                  &pPkgInfo,
                  &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwError == ERROR_TDNF_NO_MATCH && !*ppszPackageNameSpecs)
    {
        dwError = 0;
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

uint32_t
TDNFList(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
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

    dwError = SolvCreateQuery(pTdnf->pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetQueryResult(pQuery, &pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPopulatePkgInfoArray(
                  pTdnf->pSack,
                  pPkgList,
                  DETAIL_LIST,
                  &pPkgInfo,
                  &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwError == ERROR_TDNF_NO_MATCH && !*ppszPackageNameSpecs)
    {
        dwError = 0;
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

uint32_t
TDNFMakeCache(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Pass in clean metadata as 1
    dwError = TDNFRefreshSack(pTdnf, NULL, 1);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
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

    if(!pArgs || !ppTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF), (void**)&pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCloneCmdArgs(pArgs, &pTdnf->pArgs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfig(
                  pTdnf,
                  pTdnf->pArgs->pszConfFile,
                  TDNF_CONF_GROUP);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvInitSack(
                  &pSack,
                  pTdnf->pConf->pszCacheDir,
                  pTdnf->pArgs->pszInstallRoot);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFLoadRepoData(
                  pTdnf,
                  REPOLISTFILTER_ALL,
                  &pTdnf->pRepos);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoListFinalize(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRefreshSack(
                  pTdnf,
                  pSack,
                  pTdnf->pArgs->nRefresh);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pSack = pSack;
    *ppTdnf = pTdnf;

cleanup:
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

    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;
    int nAdd = 0;

    if(!pTdnf || !pTdnf->pRepos || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        nAdd = 0;
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
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !ppSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_init(&queueGoal);
    
    if(nAlterType == ALTER_AUTOERASE)
    {
        dwError = ERROR_TDNF_AUTOERASE_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }


    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&ppszPkgsNotResolved);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPrepareAllPackages(
                  pTdnf,
                  &nAlterType,
                  ppszPkgsNotResolved,
                  &queueGoal);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGoal(
                  pTdnf,
                  &queueGoal,
                  &pSolvedPkgInfo,
                  nAlterType);
    BAIL_ON_TDNF_ERROR(dwError);

    pPkgInfo = pSolvedPkgInfo->pPkgsToRemove;
    while(pPkgInfo != NULL)
    {
        if(pPkgInfo->pszName != NULL &&
           strcmp(pPkgInfo->pszName, TDNF_NAME) == 0)
        {
            dwError = ERROR_TDNF_SELF_ERASE;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pPkgInfo = pPkgInfo->pNext;
    }

    dwError = TDNFCheckProtectedPkgs(pSolvedPkgInfo);
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
    *ppSolvedPkgInfo = pSolvedPkgInfo;

cleanup:
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
    if(ppszPkgsNotResolved)
    {
        TDNFFreeStringArray(ppszPkgsNotResolved);
    }
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

    for(nIndex = 0; nIndex < unCount; nIndex++)
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
    TDNF_SCOPE nScope,
    TDNF_AVAIL nAvail,
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

        for(iAdv = 0; iAdv < nCount; iAdv++)
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
        printf(
           "\n%d updates.\n", nUpdates);
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
        if(pTdnf->pArgs)
        {
            TDNFFreeCmdArgs(pTdnf->pArgs);
        }
        if(pTdnf->pSack)
        {
            SolvFreeSack(pTdnf->pSack);
        }
        TDNFFreeMemory(pTdnf);
    }
}

const char*
TDNFGetVersion(
    )
{
    return PACKAGE_VERSION;
}
