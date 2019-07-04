/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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

    dwError = TDNFRpmExecTransaction(pTdnf, pSolvedInfo);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/* Compare files by name. */
static int
FTSEntcmp(const FTSENT **ppEntFirst, const FTSENT **ppEntSecond)
{
    return strcmp((*ppEntFirst)->fts_name, (*ppEntSecond)->fts_name);
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
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem = SKIPPROBLEM_NONE;

    if(!pTdnf || !pTdnf->pArgs || !pdwSkipProblem)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!strcasecmp(pTdnf->pArgs->ppszCmds[0], "check"))
    {
      pSetOpt = pTdnf->pArgs->pSetOpt;

      while(pSetOpt)
      {
          if(pSetOpt->nType == CMDOPT_KEYVALUE &&
            !strcasecmp(pSetOpt->pszOptName, "skipconflicts"))
          {
              dwSkipProblem |= SKIPPROBLEM_CONFLICTS;
          }
          if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "skipobsoletes"))
          {
             dwSkipProblem |= SKIPPROBLEM_OBSOLETES;
          }
          pSetOpt = pSetOpt->pNext;
      }
    }
    *pdwSkipProblem = dwSkipProblem;
cleanup:
    return dwError;

error:
    if(pdwSkipProblem)
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
    int i = 0;
    FTS* pDir = NULL;
    FTSENT* pEnt = NULL;
    HySack hSack = NULL;
    HyPackage hPkg = NULL;
    HyGoal hGoal = NULL;
    HyPackageList hPkgList = NULL;
    int nLen = 0;
    int nLenRpmExt = 0;
    int nIsDir = 0;
    char* pszLocalPathCopy = NULL;
    char *pszPathlist[2] = {NULL, NULL};
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem = SKIPPROBLEM_NONE;

    if(!pTdnf || !pszLocalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFIsDir(pszLocalPath, &nIsDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!nIsDir)
    {
        dwError = ENOTDIR;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszLocalPath, &pszLocalPathCopy);
    BAIL_ON_TDNF_ERROR(dwError);
    pszPathlist[0] = pszLocalPathCopy;

    pDir = fts_open(pszPathlist, FTS_LOGICAL | FTS_NOSTAT, FTSEntcmp);
    if(pDir == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fprintf(stdout, "Checking all packages from: %s\n", pszLocalPath);

    hSack = hy_sack_create(NULL, NULL, NULL, NULL, 0);
    if(!hSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hy_sack_create_cmdline_repo(hSack);
    hPkgList = hy_packagelist_create();
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((pEnt = fts_read(pDir)) != NULL )
    {
        if(pEnt->fts_info != FTS_F)
        {
                continue;
        }
        nLenRpmExt = strlen(TDNF_RPM_EXT);
        nLen = strlen(pEnt->fts_name);
        if (nLen <= nLenRpmExt ||
            strcmp(pEnt->fts_name + nLen - nLenRpmExt, TDNF_RPM_EXT))
        {
            continue;
        }
        hPkg = hy_sack_add_cmdline_package(hSack, pEnt->fts_path);

        if(!hPkg)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        hy_packagelist_push(hPkgList, hPkg);
        hPkg = NULL;

        printf ("%s\n", pEnt->fts_path);
    }

    fprintf(stdout, "Found %d packages\n", hy_packagelist_count(hPkgList));

    hGoal = hy_goal_create(hSack);
    if(!hGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    FOR_PACKAGELIST(hPkg, hPkgList, i)
    {
        dwError = hy_goal_install(hGoal, hPkg);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = hy_goal_run_flags(hGoal, HY_ALLOW_UNINSTALL);
    if(dwError)
    {
        TDNFGetSkipProblemOption(pTdnf, &dwSkipProblem);
        TDNFGoalReportProblems(hGoal, dwSkipProblem);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

cleanup:
    if(pDir)
    {
        fts_close(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszLocalPathCopy);
    if(hGoal)
    {
        hy_goal_free(hGoal);
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hSack)
    {
        hy_sack_free(hSack);
    }
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

    if(!pTdnf || !ppszPackageNameSpecs || !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

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

cleanup:
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
    goto cleanup;
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
            dwError = TDNFRepoRemoveCache(pTdnf, *ppszReposUsed);
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

    if(!pTdnf || !pdwCount)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = hy_sack_count(pTdnf->hSack);
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
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !pdwCount || !ppPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(hQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyPackageFilter(hQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFPopulatePkgInfoArray(
                  hPkgList,
                  DETAIL_INFO,
                  &pPkgInfo,
                  &dwCount);
    if(dwError == ERROR_TDNF_NO_MATCH && !*ppszPackageNameSpecs)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
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
    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !ppszPackageNameSpecs || !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(hQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFApplyPackageFilter(hQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFPopulatePkgInfoArray(
                  hPkgList,
                  DETAIL_LIST,
                  &pPkgInfo,
                  &dwCount);
    if(dwError == ERROR_TDNF_NO_MATCH && !*ppszPackageNameSpecs)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
    *pdwCount = dwCount;

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
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
    dwError = TDNFRefreshSack(pTdnf, 1);
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

    if(!pArgs || !ppTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                1,
                sizeof(TDNF),
                (void**)&pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCloneCmdArgs(pArgs, &pTdnf->pArgs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfig(
                  pTdnf,
                  pTdnf->pArgs->pszConfFile,
                  TDNF_CONF_GROUP);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFLoadRepoData(
                  pTdnf,
                  REPOLISTFILTER_ALL,
                  &pTdnf->pRepos);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoListFinalize(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRefreshSack(pTdnf, pTdnf->pArgs->nRefresh);
    BAIL_ON_TDNF_ERROR(dwError);

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

    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyReldep hReldep = NULL;

    int nFlag = HY_EQ;

    if(!pTdnf || IsNullOrEmptyString(pszSpec))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    hReldep = hy_reldep_create(pTdnf->hSack, pszSpec, HY_EQ, NULL);
    if(hReldep)
    {
        dwError = hy_query_filter_provides(hQuery, HY_EQ, pszSpec, NULL);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }
    else
    {
        if(TDNFIsGlob(pszSpec))
        {
            nFlag = HY_GLOB;
        }
        dwError = hy_query_filter(hQuery, HY_PKG_FILE, nFlag, pszSpec);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
      dwError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFPopulatePkgInfos(hPkgList, &pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    return dwError;

error:
    if(ppPkgInfo)
    {
      *ppPkgInfo = NULL;
    }
    TDNFFreePackageInfo(pPkgInfo);
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

    HyPackageList hPkgListGoal = NULL;

    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;

    if(!pTdnf || !ppSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_AUTOERASE)
    {
        dwError = ERROR_TDNF_AUTOERASE_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFAddExcludes(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_SOLVED_PKG_INFO),
                  (void**)&pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pSolvedPkgInfo->nAlterType = nAlterType;

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&pSolvedPkgInfo->ppszPkgsNotResolved);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  pTdnf->pArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&pSolvedPkgInfo->ppszPkgsNotInstalled);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPrepareAllPackages(
                  pTdnf,
                  pSolvedPkgInfo,
                  &hPkgListGoal);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGoal(
                  pTdnf,
                  hPkgListGoal,
                  pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

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

    *ppSolvedPkgInfo = pSolvedPkgInfo;

cleanup:
    if(hPkgListGoal)
    {
        hy_packagelist_free(hPkgListGoal);
    }
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
    HyQuery hQuery = NULL;
    HyPackage hPkg = NULL;
    HyPackageList hAccumPkgList = NULL;
    uint32_t unError = 0;
    uint32_t unCount = 0;
    int nIndex = 0;
    int nStartArgIndex = 1;
    const char* pszFirstParam = NULL;
    int bSearchAll = false;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !pCmdArgs || !ppPkgInfo || !punCount)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(unError);
    }

    hAccumPkgList = hy_packagelist_create();
    if(!hAccumPkgList)
    {
        unError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    if(pCmdArgs->nCmdCount > 1)
    {
        pszFirstParam = pCmdArgs->ppszCmds[1];
        if(!strncasecmp(pszFirstParam, "all", 3))
        {
            bSearchAll = true;
            nStartArgIndex = 2;
        }
    }

    unError = hy_sack_load_system_repo(pTdnf->hSack, NULL, 0);
    BAIL_ON_TDNF_HAWKEY_ERROR(unError);

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
      unError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    unError = TDNFQueryTerms(
        hAccumPkgList,
        pCmdArgs,
        hQuery,
        nStartArgIndex,
        QueryTermsInNameSummary);

    BAIL_ON_TDNF_ERROR(unError);

    unCount = hy_packagelist_count(hAccumPkgList);

    // Search more if nothing found or 'all' was requested.
    if (bSearchAll == true || unCount == 0)
    {
        unError = TDNFQueryTerms(
            hAccumPkgList,
            pCmdArgs,
            hQuery,
            nStartArgIndex,
            QueryTermsInDescUrl);

        BAIL_ON_TDNF_ERROR(unError);
    }

    unCount = hy_packagelist_count(hAccumPkgList);

    if (unCount < 1)
    {
        unError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(unError);
    }

    unError = TDNFAllocateMemory(
                  unCount,
                  sizeof(TDNF_PKG_INFO),
                  (void**)&pPkgInfo);

    BAIL_ON_TDNF_ERROR(unError);

    FOR_PACKAGELIST(hPkg, hAccumPkgList, nIndex)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];
        unError = TDNFSafeAllocateString(hy_package_get_name(hPkg), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(unError);

        unError = TDNFSafeAllocateString(hy_package_get_summary(hPkg),
                      &pPkg->pszSummary);
        BAIL_ON_TDNF_ERROR(unError);
    }

    *ppPkgInfo = pPkgInfo;
    *punCount = unCount;

cleanup:
    if (hAccumPkgList != NULL)
    {
        hy_packagelist_free(hAccumPkgList);
    }

    if (hQuery != NULL)
    {
        hy_query_free(hQuery);
    }

    return unError;
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
    int nCount = 0;
    int iPkg = 0;
    int iAdv = 0;
    int nPkgCount = 0;

    PTDNF_UPDATEINFO pUpdateInfos = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;

    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    HyAdvisoryList hAdvList = NULL;

    char*  pszSeverity = NULL;
    uint32_t dwSecurity = 0;
    int nUpdates = 0;

    if(!pTdnf || !ppszPackageNameSpecs || !ppUpdateInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgList, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    nPkgCount = hy_packagelist_count(hPkgList);
    if(nPkgCount == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetSecuritySeverityOption(
                  pTdnf,
                  &dwSecurity,
                  &pszSeverity);
    BAIL_ON_TDNF_ERROR(dwError);

    FOR_PACKAGELIST(hPkg, hPkgList, iPkg)
    {
        hAdvList = hy_package_get_advisories(hPkg, HY_GT);
        if(!hAdvList)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        nCount = hy_advisorylist_count(hAdvList);
        for(iAdv = 0; iAdv < nCount; iAdv++)
        {
            dwError = TDNFGetOneUpdateinfo(
                          hAdvList,
                          iAdv,
                          dwSecurity,
                          pszSeverity,
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
        hy_advisorylist_free(hAdvList);
        hAdvList = NULL;
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
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    if(hAdvList)
    {
        hy_advisorylist_free(hAdvList);
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
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
        if(pTdnf->hGoal)
        {
            hy_goal_free(pTdnf->hGoal);
        }
        if(pTdnf->pRepos)
        {
            TDNFFreeReposInternal(pTdnf->pRepos);
        }
        if(pTdnf->hSack)
        {
            hy_sack_free(pTdnf->hSack);
        }
        if(pTdnf->pConf)
        {
            TDNFFreeConfig(pTdnf->pConf);
        }
        if(pTdnf->pArgs)
        {
            TDNFFreeCmdArgs(pTdnf->pArgs);
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
