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

//check a local rpm folder for dependency issues.
uint32_t
TDNFCheckLocalPackages(
    PTDNF pTdnf,
    const char* pszLocalPath
    )
{
    uint32_t dwError = 0;
    int i = 0;
    char* pszRPMPath = NULL;
    const char* pszFile = NULL;
    GDir* pDir = NULL;
    HySack hSack = NULL;
    HyPackage hPkg = NULL;
    HyGoal hGoal = NULL;
    HyPackageList hPkgList = NULL;

    if(!pTdnf || !pszLocalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pDir = g_dir_open(pszLocalPath, 0, NULL);
    if(!pDir)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fprintf(stdout, "Checking all packages from: %s\n", pszLocalPath);

    hSack = hy_sack_create(NULL, NULL, NULL, 0);
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

    while ((pszFile = g_dir_read_name (pDir)) != NULL)
    {
        if (!g_str_has_suffix (pszFile, TDNF_RPM_EXT))
        {
            continue;
        }
        pszRPMPath = g_build_filename(pszLocalPath, pszFile, NULL);
        hPkg = hy_sack_add_cmdline_package(hSack, pszRPMPath);

        g_free(pszRPMPath);
        pszRPMPath = NULL;

        if(!hPkg)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER; 
            BAIL_ON_TDNF_ERROR(dwError);
        }
        hy_packagelist_push(hPkgList, hPkg);
        hPkg = NULL;
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
        TDNFGoalReportProblems(hGoal);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

cleanup:
    if(pDir)
    {
        g_dir_close(pDir);
    }
    if(pszRPMPath)
    {
        g_free(pszRPMPath);
    }
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
    PTDNF_CLEAN_INFO pCleanInfo = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    //Do clean metadata and refresh
    dwError = TDNFClean(pTdnf, CLEANTYPE_ALL, &pCleanInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRefreshCache(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pCleanInfo)
    {
        TDNFFreeCleanInfo(pCleanInfo);
    }
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
    HyRepo hRepo = NULL;
    int nYumFlags = HY_LOAD_FILELISTS | HY_LOAD_UPDATEINFO;

    if(!pArgs || !ppTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                sizeof(TDNF),
                (void**)&pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCloneCmdArgs(pArgs, &pTdnf->pArgs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfig(TDNF_CONF_FILE, TDNF_CONF_GROUP, &pTdnf->pConf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFLoadRepoData(
                  pTdnf->pConf,
                  REPOLISTFILTER_ENABLED,
                  &pTdnf->pRepos);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFInitSack(pTdnf, &pTdnf->hSack, HY_LOAD_FILELISTS);
    BAIL_ON_TDNF_ERROR(dwError);

    //If there is an empty repo directory, do nothing
    if(pTdnf->pRepos)
    {
        PTDNF_REPO_DATA pTempRepo = pTdnf->pRepos;
        while(pTempRepo)
        {
            if(pTempRepo->nEnabled)
            {
                dwError = TDNFInitRepo(pTdnf, pTempRepo, &hRepo);
                if(dwError)
                {
                    if(pTempRepo->nSkipIfUnavailable)
                    {
                        pTempRepo->nEnabled = 0;
                        fprintf(stderr, "Disabling Repo: '%s'\n", pTempRepo->pszName);

                        dwError = 0;
                    }
                }
                BAIL_ON_TDNF_ERROR(dwError);

                if(pTempRepo->nEnabled)
                {
                    pTempRepo->hRepo = hRepo;

                    dwError = TDNFLoadYumRepo(pTdnf->hSack, hRepo, nYumFlags);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
            }
            pTempRepo = pTempRepo->pNext;
        }
    }
    
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

    if(!pTdnf || !pTdnf->pConf || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFLoadRepoData(pTdnf->pConf, nFilter, &pReposAll);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppReposAll = pReposAll;

cleanup:
    return dwError;

error:
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

    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyPackageList hPkgListGoal = NULL;
    HySelector hSelector = NULL;

    const char* pszPkgName = NULL;

    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;

    if(!pTdnf || !ppSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_FAILED;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    //TODO: support multiple packages
    pszPkgName = pTdnf->pArgs->ppszCmds[1];

    dwError = TDNFAllocateMemory(
                sizeof(TDNF_SOLVED_PKG_INFO),
                (void**)&pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pSolvedPkgInfo->nAlterType = nAlterType;

    if(nAlterType == ALTER_AUTOERASE)
    {
        dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pTdnf->hSack,
                      pszPkgName,
                      &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFGetMatchingInstalledAndAvailable(
                      pTdnf,
                      nAlterType,
                      pszPkgName,
                      pSolvedPkgInfo,
                      &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFGetSelector(
                      pTdnf,
                      pszPkgName,
                      &hSelector);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(hSelector != NULL)
    {
        hPkgListGoal = hy_selector_matches(hSelector);
    }
    if(hy_packagelist_count(hPkgListGoal) > 0)
    {
        dwError = TDNFGoal(
                      pTdnf,
                      hPkgListGoal,
                      hSelector,
                      nAlterType,
                      pSolvedPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateMemory(
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pSolvedPkgInfo->pPkgsNotAvailable);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(
                      pszPkgName,
                      &pSolvedPkgInfo->pPkgsNotAvailable->pszName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pSolvedPkgInfo->nNeedAction = 
        pSolvedPkgInfo->pPkgsToInstall ||
        pSolvedPkgInfo->pPkgsToUpgrade ||
        pSolvedPkgInfo->pPkgsToDowngrade ||
        pSolvedPkgInfo->pPkgsToRemove  ||
        pSolvedPkgInfo->pPkgsUnNeeded ||
        pSolvedPkgInfo->pPkgsToReinstall;

    *ppSolvedPkgInfo = pSolvedPkgInfo;

cleanup:
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    TDNF_SAFE_FREE_PKGLIST(hPkgList);
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
    HySack hSack = NULL;
    HyQuery hQuery = NULL;
    HyPackage hPkg = NULL;
    HyPackageList hAccumPkgList = NULL;
    uint32_t unError = 0;
    uint32_t unCount = 0;
    int nIndex = 0;
    int nStartArgIndex = 1;
    const char* pszFirstParam = NULL;
    bool bSearchAll = false;
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

    hSack = hy_sack_create(NULL, NULL, NULL, 0);
    if(!hSack)
    {
        unError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    unError = hy_sack_load_system_repo(hSack, NULL, 0);
    BAIL_ON_TDNF_HAWKEY_ERROR(unError);

    hQuery = hy_query_create(hSack);
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
                sizeof(TDNF_PKG_INFO) * unCount,
                (void**)&pPkgInfo);

    BAIL_ON_TDNF_ERROR(unError);

    FOR_PACKAGELIST(hPkg, hAccumPkgList, nIndex)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];
        unError = TDNFSafeAllocateString(hy_package_get_name(hPkg), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(unError);

        unError = TDNFSafeAllocateString(hy_package_get_summary(hPkg), &pPkg->pszSummary);
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

    if (hSack != NULL)
    {
        hy_sack_free(hSack);
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

    PTDNF_UPDATEINFO pUpdateInfos = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;
    const char* pszTemp = NULL;

    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    HyAdvisoryList hAdvList = NULL;
    HyAdvisory hAdv = NULL;

    if(!pTdnf || !ppszPackageNameSpecs || !ppUpdateInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgList);
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
            dwError = TDNFAllocateMemory(
                          sizeof(TDNF_UPDATEINFO),
                          (void**)&pInfo);
            BAIL_ON_TDNF_ERROR(dwError);

            hAdv = hy_advisorylist_get_clone(hAdvList, iAdv);
            if(!hAdv)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }

            pInfo->nType = hy_advisory_get_type(hAdv);
            pszTemp = hy_advisory_get_id(hAdv);
            if(pszTemp)
            {
                dwError = TDNFAllocateString(pszTemp, &pInfo->pszID);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFGetUpdateInfoPackages(hAdv, &pInfo->pPackages);
            BAIL_ON_TDNF_ERROR(dwError);

            hy_advisory_free(hAdv);
            hAdv = NULL;

            pInfo->pNext = pUpdateInfos;
            pUpdateInfos = pInfo;
            pInfo = NULL;
        }
        hy_advisorylist_free(hAdvList);
        hAdvList = NULL;
    }

    *ppUpdateInfo = pUpdateInfos;

cleanup:
    if(hAdv)
    {
        hy_advisory_free(hAdv);
    }
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

void
TDNFFreeCmdArgs(
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    int nIndex = 0;
    if(pCmdArgs)
    {
        for(nIndex = 0; nIndex < pCmdArgs->nCmdCount; ++nIndex)
        {
            TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds[nIndex]);
        }
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds);
    }
    TDNF_SAFE_FREE_MEMORY(pCmdArgs);
}

