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
    //int i = 0;
    char* pszRPMPath = NULL;
    const char* pszFile = NULL;
    GDir* pDir = NULL;
    PSolvSack hSack = NULL;
    //PPackage hPkg = NULL;
    //PGoal hGoal = NULL;
    //PPackageList hPkgList = NULL;

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

    //hSack = hy_sack_create(NULL, NULL, NULL, 0);
    //if(!hSack)
    //{
    //    dwError = ERROR_TDNF_INVALID_PARAMETER;
    //    BAIL_ON_TDNF_ERROR(dwError);
    //}

    //hy_sack_create_cmdline_repo(hSack);
    //hPkgList = hy_packagelist_create();
    //if(!hPkgList)
    //{
    //    dwError = ERROR_TDNF_INVALID_PARAMETER;
    //    BAIL_ON_TDNF_ERROR(dwError);
    //}

    while ((pszFile = g_dir_read_name (pDir)) != NULL)
    {
        if (!g_str_has_suffix (pszFile, TDNF_RPM_EXT))
        {
            continue;
        }
        pszRPMPath = g_build_filename(pszLocalPath, pszFile, NULL);
        //hPkg = hy_sack_add_cmdline_package(hSack, pszRPMPath);

        //if(!hPkg)
        //{
        //    dwError = ERROR_TDNF_INVALID_PARAMETER; 
        //    BAIL_ON_TDNF_ERROR(dwError);
        //}
        //hy_packagelist_push(hPkgList, hPkg);
        //hPkg = NULL;

        g_free(pszRPMPath);
        pszRPMPath = NULL;
    }

    //fprintf(stdout, "Found %d packages\n", hy_packagelist_count(hPkgList));

    //hGoal = hy_goal_create(hSack);
    //if(!hGoal)
    //{
    //    dwError = ERROR_TDNF_INVALID_PARAMETER;
    //    BAIL_ON_TDNF_ERROR(dwError);
    //}

    //FOR_PACKAGELIST(hPkg, hPkgList, i)
    //{
    //    dwError = hy_goal_install(hGoal, hPkg);
    //    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    //}
    
    //dwError = hy_goal_run_flags(hGoal, HY_ALLOW_UNINSTALL);
    //if(dwError)
    //{
    //    TDNFGoalReportProblems(hGoal);
    //    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    //}

cleanup:
    if(pDir)
    {
        g_dir_close(pDir);
    }
    if(pszRPMPath)
    {
        g_free(pszRPMPath);
    }
    //if(hGoal)
    //{
    //    hy_goal_free(hGoal);
    //} 
    //if(hPkgList)
    //{
    //    hy_packagelist_free(hPkgList);
    //} 
    if(hSack)
    {
        //hy_sack_free(hSack);
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

    dwCount = SolvCountPackages(pTdnf->pSack);
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

    if(!pTdnf || !pdwCount || !ppPkgInfo)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pTdnf->pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    pPkgList = SolvCreatePackageList();
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPopulatePkgInfoArray(
                    pTdnf->pSack,
                    pPkgList,
                    DETAIL_INFO,
                    &pPkgInfo,
                    &dwCount);
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

    if(!pTdnf || !ppszPackageNameSpecs || !ppPkgInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pTdnf->pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFApplyScopeFilter(pQuery, nScope);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyPackageFilter(pQuery, ppszPackageNameSpecs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    pPkgList = SolvCreatePackageList();
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    
    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);


    dwError = TDNFPopulatePkgInfoArray(
                  pTdnf->pSack,
                  pPkgList,
                  DETAIL_LIST,
                  &pPkgInfo,
                  &dwCount);
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
    PSolvSack pSack = NULL;

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

    dwError = TDNFReadConfig(pTdnf,
                  pTdnf->pArgs->pszConfFile,
                  TDNF_CONF_GROUP);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvInitSack(&pSack, pTdnf->pConf->pszCacheDir, pTdnf->pArgs->pszInstallRoot);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pSack = pSack;
    
    dwError = TDNFLoadRepoData(
                  pTdnf,
                  REPOLISTFILTER_ENABLED,
                  &pTdnf->pRepos);
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

    //int nFlag = HY_EQ;

    if(!pTdnf || IsNullOrEmptyString(pszSpec))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppPkgInfo = pPkgInfo;
cleanup:
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

    dwError = TDNFLoadRepoData(pTdnf, nFilter, &pReposAll);
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
    PSolvQuery pQuery = NULL;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;
    char** ppszPackages = NULL;
    //int nCmdIndex = 0;

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

    dwError = TDNFValidateCmdArgs(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    pQuery = SolvCreateQuery(pTdnf->pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->pArgs->nCmdCount > 1 && nAlterType != ALTER_REINSTALL)
    {
        ppszPackages = pTdnf->pArgs->ppszCmds;
        dwError = SolvApplyPackageFilter(pQuery, ++ppszPackages);
    }
    switch(nAlterType)
    {
        case ALTER_ERASE:
            dwError = TDNFApplyScopeFilter(pQuery, SCOPE_INSTALLED);
            BAIL_ON_TDNF_ERROR(dwError);
            dwError = SolvApplyEraseQuery(pQuery);
            break;
        case ALTER_INSTALL:
            dwError = TDNFApplyScopeFilter(pQuery, SCOPE_AVAILABLE);
            dwError = SolvApplyInstallQuery(pQuery);
            break;
        case ALTER_UPGRADE:
        case ALTER_UPGRADEALL:
            dwError = SolvApplyUpdateQuery(pQuery);
            break;
        case ALTER_DOWNGRADE:

            dwError = TDNFMatchForDowngrade(pTdnf, pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            dwError = SolvApplyDowngradeQuery(pQuery);
            break;
        case ALTER_DOWNGRADEALL:
            dwError = TDNFMatchForDowngradeAll(pTdnf, pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            dwError = SolvApplyDowngradeQuery(pQuery);
            break;
        case ALTER_DISTRO_SYNC:
            dwError = SolvApplyDistroSyncQuery(pQuery);
            break;
        case ALTER_REINSTALL:
            dwError = TDNFMatchForReinstall(pTdnf, pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            dwError = SolvApplyReinstallQuery(pQuery);
            break;
        default:
            break;
    }

    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;//ERROR_TDNF_NO_MATCH;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetAllResultsIgnoreNoData(nAlterType, pQuery, &pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);
    pSolvedPkgInfo->nAlterType = nAlterType;

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
    uint32_t dwError = 0;
    int nStartArgIndex = 1;
    PSolvQuery pQuery = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PSolvPackageList pPkgList = NULL;
    int nIndex = 0;
    int unCount  = 0;
    Id  pkgId = 0;
    if(!pTdnf || !pCmdArgs || !ppPkgInfo || !punCount)
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

    pQuery = SolvCreateQuery(pTdnf->pSack);
    SolvApplySearch(pQuery,
                        pCmdArgs->ppszCmds,
                        nStartArgIndex,
                        pCmdArgs->nCmdCount);

    pPkgList = SolvCreatePackageList();
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetSearchResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    unCount = SolvGetPackageListSize(pPkgList);

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
        dwError = SolvGetPackageId(pPkgList, nIndex, &pkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];
        dwError = TDNFSafeAllocateString(SolvGetPkgNameFromId(pTdnf->pSack, pkgId), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(SolvGetPkgSummaryFromId(pTdnf->pSack, pkgId), &pPkg->pszSummary);
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
    return 1;
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
            TDNFFreeRepos(pTdnf->pRepos);
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
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszInstallRoot);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszConfFile);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszReleaseVer);

        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pSetOpt);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs);
    }
}

const char*
TDNFGetVersion(
    )
{
    return PACKAGE_VERSION;
}

void
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    )
{
    PTDNF_CMD_OPT pCmdOptTemp = NULL;
    while(pCmdOpt)
    {
        TDNF_SAFE_FREE_MEMORY(pCmdOpt->pszOptName);
        TDNF_SAFE_FREE_MEMORY(pCmdOpt->pszOptValue);
        pCmdOptTemp = pCmdOpt->pNext;
        TDNF_SAFE_FREE_MEMORY(pCmdOpt);
        pCmdOpt = pCmdOptTemp;
    }
}
