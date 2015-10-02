/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : init.c
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
TDNFLoadYumRepo(
    HySack hSack,
    HyRepo hRepo,
    int nFlags
    )
{
    uint32_t dwError = 0;

    if(!hSack || !hRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = hy_sack_load_yum_repo(hSack, hRepo, nFlags);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

cleanup:
    return dwError;

error:
    if(hSack)
    {
        hy_sack_free(hSack);
    }
    goto cleanup;
}

uint32_t
TDNFInitSack(
    PTDNF pTdnf,
    HySack* phSack,
    int nFlags
    )
{
    uint32_t dwError = 0;
    HySack hSack = NULL;
    char* pszHawkeyCacheDir = NULL;

    if(!pTdnf || !pTdnf->pConf || !phSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszHawkeyCacheDir = pTdnf->pConf->pszCacheDir;

    hSack = hy_sack_create(pszHawkeyCacheDir, NULL, NULL, "/", 0);
    if(!hSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = hy_sack_load_system_repo(hSack, NULL, nFlags);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    *phSack = hSack;
cleanup:
    return dwError;

error:
    if(hSack)
    {
        hy_sack_free(hSack);
    }
    if(phSack)
    {
        *phSack = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;

    dwError = TDNFAllocateMemory(
                            sizeof(TDNF_CMD_ARGS),
                            (void**)&pCmdArgs);
    BAIL_ON_TDNF_ERROR(dwError);

    pCmdArgs->nAllowErasing  = pCmdArgsIn->nAllowErasing;
    pCmdArgs->nAssumeNo      = pCmdArgsIn->nAssumeNo;
    pCmdArgs->nAssumeYes     = pCmdArgsIn->nAssumeYes;
    pCmdArgs->nBest          = pCmdArgsIn->nBest;
    pCmdArgs->nCacheOnly     = pCmdArgsIn->nCacheOnly;
    pCmdArgs->nDebugSolver   = pCmdArgsIn->nDebugSolver;
    pCmdArgs->nNoGPGCheck    = pCmdArgsIn->nNoGPGCheck;
    pCmdArgs->nRefresh       = pCmdArgsIn->nRefresh;
    pCmdArgs->nRpmVerbosity  = pCmdArgsIn->nRpmVerbosity;
    pCmdArgs->nShowDuplicates= pCmdArgsIn->nShowDuplicates;
    pCmdArgs->nShowHelp      = pCmdArgsIn->nShowHelp;
    pCmdArgs->nShowVersion   = pCmdArgsIn->nShowVersion;
    pCmdArgs->nVerbose       = pCmdArgsIn->nVerbose;
    pCmdArgs->nIPv4          = pCmdArgsIn->nIPv4;
    pCmdArgs->nIPv6          = pCmdArgsIn->nIPv6;

    pCmdArgs->nCmdCount = pCmdArgsIn->nCmdCount;
    dwError = TDNFAllocateMemory(
                            pCmdArgs->nCmdCount * sizeof(char*),
                            (void**)&pCmdArgs->ppszCmds
                            );
    BAIL_ON_TDNF_ERROR(dwError);
        
    for(nIndex = 0; nIndex < pCmdArgs->nCmdCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                         pCmdArgsIn->ppszCmds[nIndex],
                         &pCmdArgs->ppszCmds[nIndex]);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppCmdArgs = pCmdArgs;

cleanup:
    return dwError;

error:
    if(ppCmdArgs)
    {
        *ppCmdArgs = NULL;
    }
    TDNFFreeCmdArgs(pCmdArgs);
    goto cleanup;
}

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
    int nCleanMetadata
    )
{
    uint32_t dwError = 0;
    HyRepo hRepo = NULL;
    int nYumFlags = HY_LOAD_FILELISTS | HY_LOAD_UPDATEINFO;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pTdnf->hSack)
    {
        hy_sack_free(pTdnf->hSack);
        pTdnf->hSack = NULL;
    }

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
                if(nCleanMetadata)
                {
                    fprintf(stdout,
                            "Refreshing metadata for: '%s'\n",
                            pTempRepo->pszName);
                    dwError = TDNFRepoRemoveCache(pTdnf, pTempRepo->pszId);
                    if(dwError == ERROR_TDNF_FILE_NOT_FOUND)
                    {
                        dwError = 0;//Ignore non existent folders
                    }
                    BAIL_ON_TDNF_ERROR(dwError);
                }

                dwError = TDNFInitRepo(pTdnf, pTempRepo, &hRepo);
                if(dwError)
                {
                    if(pTempRepo->nSkipIfUnavailable)
                    {
                        pTempRepo->nEnabled = 0;
                        fprintf(stdout, "Disabling Repo: '%s'\n", pTempRepo->pszName);

                        dwError = 0;
                    }
                }
                BAIL_ON_TDNF_ERROR(dwError);

                if(pTempRepo->nEnabled)
                {
                    if(pTempRepo->hRepo)
                    {
                        hy_repo_free(pTempRepo->hRepo);
                        pTempRepo->hRepo = NULL;
                    }
                    pTempRepo->hRepo = hRepo;

                    dwError = TDNFLoadYumRepo(pTdnf->hSack, hRepo, nYumFlags);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
            }
            pTempRepo = pTempRepo->pNext;
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
