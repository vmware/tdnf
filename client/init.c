/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : init.c
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
#include "includes.h"

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

    hSack = hy_sack_create(pszHawkeyCacheDir, NULL, "/", 0);
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
