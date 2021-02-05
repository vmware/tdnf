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
    pCmdArgs->nCacheOnly     = pCmdArgsIn->nCacheOnly;
    pCmdArgs->nRpmVerbosity  = pCmdArgsIn->nRpmVerbosity;
    pCmdArgs->nShowDuplicates= pCmdArgsIn->nShowDuplicates;
    pCmdArgs->nShowHelp      = pCmdArgsIn->nShowHelp;
    pCmdArgs->nShowVersion   = pCmdArgsIn->nShowVersion;
    pCmdArgs->nVerbose       = pCmdArgsIn->nVerbose;
    pCmdArgs->nIPv4          = pCmdArgsIn->nIPv4;
    pCmdArgs->nIPv6          = pCmdArgsIn->nIPv6;
    pCmdArgs->nDisableExcludes = pCmdArgsIn->nDisableExcludes;
    pCmdArgs->nDownloadOnly  = pCmdArgsIn->nDownloadOnly;

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

    if(!IsNullOrEmptyString(pCmdArgsIn->pszDownloadDir))
    {
        dwError = TDNFAllocateString(
                      pCmdArgsIn->pszDownloadDir,
                      &pCmdArgs->pszDownloadDir);
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
    else /* if there are no setopt values, prime it to ensure non null */
    {
        dwError = AddSetOptWithValues(
                      pCmdArgs,
                      CMDOPT_KEYVALUE,
                      TDNF_SETOPT_NAME_DUMMY,
                      TDNF_SETOPT_VALUE_DUMMY);
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

        if (pCmdOptCurrent->nType != CMDOPT_CURL_INIT_CB)
        {
           dwError = TDNFAllocateString(pCmdOptIn->pszOptValue,
                                        &pCmdOptCurrent->pszOptValue);
           BAIL_ON_TDNF_ERROR(dwError);
        }
        else
        {
           pCmdOptCurrent->pfnCurlConfigCB = pCmdOptIn->pfnCurlConfigCB;
        }

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
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nCleanMetadata == 1 && pTdnf->pArgs)
    {
        pTdnf->pArgs->nRefresh = 1;
    }

    for(pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
    {
        /* skip the @cmdline repo - options do not apply, and it is
           initialized. */
        if ((strcmp(pRepo->pszName, "@cmdline") == 0) ||
            (!pRepo->nEnabled))
        {
            continue;
        }

        nMetadataExpired = 0;
        //Check if expired since last sync per metadata_expire
        if(pRepo->lMetadataExpire >= 0)
        {
            dwError = TDNFAllocateStringPrintf(
                          &pszRepoCacheDir,
                          "%s/%s",
                          pTdnf->pConf->pszCacheDir,
                          pRepo->pszId);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFShouldSyncMetadata(
                          pszRepoCacheDir,
                          pRepo->lMetadataExpire,
                          &nMetadataExpired);
            BAIL_ON_TDNF_ERROR(dwError);

            TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
            pszRepoCacheDir = NULL;
        }

        if(nMetadataExpired)
        {
            dwError = TDNFRepoRemoveCache(pTdnf, pRepo->pszId);
            if(dwError == ERROR_TDNF_FILE_NOT_FOUND)
            {
                dwError = 0;//Ignore non existent folders
            }
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFRemoveSolvCache(pTdnf, pRepo->pszId);
            if(dwError == ERROR_TDNF_FILE_NOT_FOUND)
            {
                dwError = 0;//Ignore non existent folders
            }
            BAIL_ON_TDNF_ERROR(dwError);
        }

        if(pSack)
        {
            dwError = TDNFInitRepo(pTdnf, pRepo, pSack);
        }
        if(dwError && pRepo->nSkipIfUnavailable)
        {
            pRepo->nEnabled = 0;
            pr_info("Disabling Repo: '%s'\n",
                    pRepo->pszName);
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRefresh(
    PTDNF pTdnf)
{
    uint32_t dwError = 0;
    if(!pTdnf || !pTdnf->pSack || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        return dwError;
    }
    return TDNFRefreshSack(pTdnf, pTdnf->pSack, pTdnf->pArgs->nRefresh);
}
