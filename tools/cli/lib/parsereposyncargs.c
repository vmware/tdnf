/*
 * Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFCliParseRepoSyncArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_REPOSYNC_ARGS* ppReposyncArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPOSYNC_ARGS pReposyncArgs = NULL;
    PTDNF_CMD_OPT pSetOpt = NULL;
    int i;

    if (!pArgs || !ppReposyncArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
        1,
        sizeof(TDNF_REPOSYNC_ARGS),
        (void**) &pReposyncArgs);
    BAIL_ON_CLI_ERROR(dwError);

    for (pSetOpt = pArgs->pSetOpt;
         pSetOpt;
         pSetOpt = pSetOpt->pNext)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE)
        {
            if (strcasecmp(pSetOpt->pszOptName, "arch") == 0)
            {
                if (pReposyncArgs->ppszArchs == NULL)
                {
                    TDNFAllocateMemory(TDNF_REPOSYNC_MAXARCHS+1, sizeof(char *),
                        (void **)&pReposyncArgs->ppszArchs);
                    BAIL_ON_CLI_ERROR(dwError);
                }
                for (i = 0; pReposyncArgs->ppszArchs[i] && i < TDNF_REPOSYNC_MAXARCHS; i++);
                if (i < TDNF_REPOSYNC_MAXARCHS)
                {
                    dwError = TDNFAllocateString(
                        pSetOpt->pszOptValue,
                        &(pReposyncArgs->ppszArchs[i]));
                    BAIL_ON_CLI_ERROR(dwError);
                }
            }
            else if (strcasecmp(pSetOpt->pszOptName, "delete") == 0)
            {
                pReposyncArgs->nDelete = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "download-metadata") == 0)
            {
                pReposyncArgs->nDownloadMetadata = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "gpgcheck") == 0)
            {
                pReposyncArgs->nGPGCheck = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "newest-only") == 0)
            {
                pReposyncArgs->nNewestOnly = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "norepopath") == 0)
            {
                pReposyncArgs->nNoRepoPath = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "source") == 0)
            {
                pReposyncArgs->nSourceOnly = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "urls") == 0)
            {
                pReposyncArgs->nPrintUrlsOnly = 1;
            }
            else if (strcasecmp(pSetOpt->pszOptName, "download-path") == 0)
            {
                dwError = TDNFAllocateString(
                    pSetOpt->pszOptValue,
                    &pReposyncArgs->pszDownloadPath);
                BAIL_ON_CLI_ERROR(dwError);
            }
            else if (strcasecmp(pSetOpt->pszOptName, "metadata-path") == 0)
            {
                dwError = TDNFAllocateString(
                    pSetOpt->pszOptValue,
                    &pReposyncArgs->pszMetaDataPath);
                BAIL_ON_CLI_ERROR(dwError);
            }
        }
    }
    *ppReposyncArgs = pReposyncArgs;
cleanup:
    return dwError;
error:
    if (pReposyncArgs)
    {
        TDNFCliFreeRepoSyncArgs(pReposyncArgs);
    }
    goto cleanup;
}

void
TDNFCliFreeRepoSyncArgs(
    PTDNF_REPOSYNC_ARGS pReposyncArgs
    )
{
    if(pReposyncArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pReposyncArgs->ppszArchs);
        TDNF_SAFE_FREE_MEMORY(pReposyncArgs->pszDownloadPath);
        TDNF_SAFE_FREE_MEMORY(pReposyncArgs->pszMetaDataPath);
        TDNFFreeMemory(pReposyncArgs);
    }
}

