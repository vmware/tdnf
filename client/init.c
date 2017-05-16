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
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;

    dwError = TDNFAllocateMemory(
                  1,
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
    pCmdArgs->nNoOutput      = pCmdArgsIn->nNoOutput;
    pCmdArgs->nQuiet         = pCmdArgsIn->nQuiet;
    pCmdArgs->nRefresh       = pCmdArgsIn->nRefresh;
    pCmdArgs->nRpmVerbosity  = pCmdArgsIn->nRpmVerbosity;
    pCmdArgs->nShowDuplicates= pCmdArgsIn->nShowDuplicates;
    pCmdArgs->nShowHelp      = pCmdArgsIn->nShowHelp;
    pCmdArgs->nShowVersion   = pCmdArgsIn->nShowVersion;
    pCmdArgs->nVerbose       = pCmdArgsIn->nVerbose;
    pCmdArgs->nIPv4          = pCmdArgsIn->nIPv4;
    pCmdArgs->nIPv6          = pCmdArgsIn->nIPv6;

    dwError = TDNFAllocateString(
                  pCmdArgsIn->pszInstallRoot,
                  &pCmdArgs->pszInstallRoot);
    BAIL_ON_TDNF_ERROR(dwError);

    if(IsNullOrEmptyString(pCmdArgsIn->pszConfFile))
    {
        dwError = TDNFAllocateString(
                      TDNF_CONF_FILE,
                      &pCmdArgs->pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateString(
                      pCmdArgsIn->pszConfFile,
                      &pCmdArgs->pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pCmdArgsIn->pszReleaseVer))
    {
        dwError = TDNFAllocateString(
                      pCmdArgsIn->pszReleaseVer,
                      &pCmdArgs->pszReleaseVer);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCmdArgs->nCmdCount = pCmdArgsIn->nCmdCount;
    dwError = TDNFAllocateMemory(
                  pCmdArgs->nCmdCount,
                  sizeof(char*),
                  (void**)&pCmdArgs->ppszCmds);
    BAIL_ON_TDNF_ERROR(dwError);

    for(nIndex = 0; nIndex < pCmdArgs->nCmdCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgsIn->ppszCmds[nIndex],
                      &pCmdArgs->ppszCmds[nIndex]);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pCmdArgsIn->pSetOpt)
    {
        dwError = TDNFCloneSetOpts(pCmdArgsIn->pSetOpt,
                                   &pCmdArgs->pSetOpt);
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
TDNFCloneSetOpts(
    PTDNF_CMD_OPT pCmdOptIn,
    PTDNF_CMD_OPT* ppCmdOpt
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pCmdOpt = NULL;
    PTDNF_CMD_OPT pCmdOptCurrent = NULL;
    PTDNF_CMD_OPT* ppCmdOptCurrent = NULL;

    if(!pCmdOptIn || !ppCmdOpt)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppCmdOptCurrent = &pCmdOpt;
    while(pCmdOptIn)
    {
        dwError = TDNFAllocateMemory(1,
                                     sizeof(TDNF_CMD_OPT),
                                     (void**)ppCmdOptCurrent);
        BAIL_ON_TDNF_ERROR(dwError);

        pCmdOptCurrent = *ppCmdOptCurrent;

        pCmdOptCurrent->nType = pCmdOptIn->nType;

        dwError = TDNFAllocateString(pCmdOptIn->pszOptName,
                                     &pCmdOptCurrent->pszOptName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pCmdOptIn->pszOptValue,
                                     &pCmdOptCurrent->pszOptValue);
        BAIL_ON_TDNF_ERROR(dwError);

        ppCmdOptCurrent = &(pCmdOptCurrent->pNext);
        pCmdOptIn = pCmdOptIn->pNext;
    }

    *ppCmdOpt = pCmdOpt;
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
    PSolvSack pSack,
    int nCleanMetadata
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;
    int nMetadataExpired = 0;
    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //If there is an empty repo directory, do nothing
    if(pTdnf->pRepos)
    {
        PTDNF_REPO_DATA_INTERNAL pTempRepo = pTdnf->pRepos;
        while(pTempRepo)
        {
            nMetadataExpired = 0;
            if(pTempRepo->nEnabled)
            {
                //Check if expired since last sync per metadata_expire
                if(!nCleanMetadata && pTempRepo->lMetadataExpire >= 0)
                {
                    dwError = TDNFAllocateStringPrintf(
                                  &pszRepoCacheDir,
                                  "%s/%s",
                                  pTdnf->pConf->pszCacheDir,
                                  pTempRepo->pszId);
                    BAIL_ON_TDNF_ERROR(dwError);

                    dwError = TDNFShouldSyncMetadata(
                                  pszRepoCacheDir,
                                  pTempRepo->lMetadataExpire,
                                  &nMetadataExpired);
                    BAIL_ON_TDNF_ERROR(dwError);

                    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
                    pszRepoCacheDir = NULL;
                }

                if(nCleanMetadata || nMetadataExpired)
                {
                    if(!pTdnf->pArgs->nQuiet)
                    {
                        fprintf(stdout,
                                "Refreshing metadata for: '%s'\n",
                                pTempRepo->pszName);
                    }
                    dwError = TDNFRepoRemoveCache(pTdnf, pTempRepo->pszId);
                    if(dwError == ERROR_TDNF_FILE_NOT_FOUND)
                    {
                        dwError = 0;//Ignore non existent folders
                    }
                    BAIL_ON_TDNF_ERROR(dwError);
                }

                if(pSack)
                {
                    dwError = TDNFInitRepo(pTdnf, pTempRepo, pSack);
                }
                if(dwError)
                {
                    if(pTempRepo->nSkipIfUnavailable)
                    {
                        pTempRepo->nEnabled = 0;
                        fprintf(stdout,
                                "Disabling Repo: '%s'\n",
                                pTempRepo->pszName);

                        dwError = 0;
                    }
                }
                BAIL_ON_TDNF_ERROR(dwError);
            }
            pTempRepo = pTempRepo->pNext;
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    return dwError;

error:
    goto cleanup;
}
