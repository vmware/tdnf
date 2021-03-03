/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : repolist.c
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
TDNFLoadRepoData(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA_INTERNAL* ppReposAll
    )
{
    uint32_t dwError = 0;
    char* pszRepoFilePath = NULL;
    PTDNF_REPO_DATA_INTERNAL pReposAll = NULL;
    PTDNF_REPO_DATA_INTERNAL pReposTemp = NULL;
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;
    PTDNF_CONF pConf = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    int nLen = 0;
    int nLenRepoExt = 0;

    if(!pTdnf || !pTdnf->pConf || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pConf = pTdnf->pConf;

    dwError = TDNFCreateCmdLineRepo(&pReposAll);
    BAIL_ON_TDNF_ERROR(dwError);

    pDir = opendir(pConf->pszRepoDir);
    if(pDir == NULL)
    {
        dwError = ERROR_TDNF_REPO_DIR_OPEN;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((pEnt = readdir (pDir)) != NULL )
    {
        nLen = strlen(pEnt->d_name);
        nLenRepoExt = strlen(TDNF_REPO_EXT);
        if (nLen <= nLenRepoExt ||
            strcmp(pEnt->d_name + nLen - nLenRepoExt, TDNF_REPO_EXT))
        {
            continue;
        }

        dwError = TDNFAllocateStringPrintf(
                      &pszRepoFilePath,
                      "%s/%s",
                      pConf->pszRepoDir,
                      pEnt->d_name);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFLoadReposFromFile(pTdnf, pszRepoFilePath, &pRepos);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszRepoFilePath);
        pszRepoFilePath = NULL;

        //Apply filter
        if((nFilter == REPOLISTFILTER_ENABLED && !pRepos->nEnabled) ||
           (nFilter == REPOLISTFILTER_DISABLED && pRepos->nEnabled))
        {
            TDNFFreeReposInternal(pRepos);
            pRepos = NULL;
            continue;
        }

        if(!pReposAll)
        {
            pReposAll = pRepos;
        }
        else
        {
            pReposTemp = pReposAll;
            while(pReposAll->pNext)
            {
                pReposAll = pReposAll->pNext;
            }
            pReposAll->pNext = pRepos;
            pReposAll = pReposTemp;
        }
    }

    *ppReposAll = pReposAll;
cleanup:
    if(pDir)
    {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszRepoFilePath);

    return dwError;
error:
    if(ppReposAll)
    {
        *ppReposAll = NULL;
    }
    if(pReposAll)
    {
        TDNFFreeReposInternal(pReposAll);
    }
    goto cleanup;
}

uint32_t
TDNFCreateCmdLineRepo(
    PTDNF_REPO_DATA_INTERNAL* ppRepo
    )
{
    uint32_t dwError;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;

    if(!ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_DATA_INTERNAL),
                  (void**)&pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString("@cmdline", &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString("@cmdline", &pRepo->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppRepo = pRepo;
cleanup:
    return dwError;
error:
    if(ppRepo)
    {
        *ppRepo = NULL;
    }
    if(pRepo)
    {
        TDNFFreeReposInternal(pRepo);
    }
    goto cleanup;
}

uint32_t
TDNFEventRepoReadConfigEnd(
    PTDNF pTdnf,
    PCONF_SECTION pSection
    )
{
    uint32_t dwError = 0;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf || !pSection)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    stContext.nEvent = MAKE_PLUGIN_EVENT(
                           TDNF_PLUGIN_EVENT_TYPE_REPO,
                           TDNF_PLUGIN_EVENT_STATE_READCONFIG,
                           TDNF_PLUGIN_EVENT_PHASE_END);
    dwError = TDNFAddEventDataPtr(&stContext,
                  TDNF_EVENT_ITEM_REPO_SECTION,
                  pSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPluginRaiseEvent(pTdnf, &stContext);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNFFreeEventData(stContext.pData);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFEventRepoReadConfigStart(
    PTDNF pTdnf,
    PCONF_SECTION pSection
    )
{
    uint32_t dwError = 0;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf || !pSection)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    stContext.nEvent = MAKE_PLUGIN_EVENT(
                           TDNF_PLUGIN_EVENT_TYPE_REPO,
                           TDNF_PLUGIN_EVENT_STATE_READCONFIG,
                           TDNF_PLUGIN_EVENT_PHASE_START);
    dwError = TDNFAddEventDataPtr(&stContext,
                  TDNF_EVENT_ITEM_REPO_SECTION,
                  pSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFPluginRaiseEvent(pTdnf, &stContext);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNFFreeEventData(stContext.pData);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFLoadReposFromFile(
    PTDNF pTdnf,
    char* pszRepoFile,
    PTDNF_REPO_DATA_INTERNAL* ppRepos
    )
{
    char *pszRepo = NULL;
    uint32_t dwError = 0;
    char *pszMetadataExpire = NULL;

    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;

    PCONF_DATA pData = NULL;
    PCONF_SECTION pSections = NULL;

    dwError = TDNFReadConfigFile(pszRepoFile, 0, &pData);
    BAIL_ON_TDNF_ERROR(dwError);

    pSections = pData->pSections;
    for(; pSections; pSections = pSections->pNext)
    {
        /* plugin event repo readconfig start */
        dwError = TDNFEventRepoReadConfigStart(pTdnf, pSections);
        BAIL_ON_TDNF_ERROR(dwError);

        pszRepo = pSections->pszName;

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_REPO_DATA_INTERNAL),
                      (void**)&pRepo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pszRepo, &pRepo->pszId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_ENABLED,
                      0,
                      &pRepo->nEnabled);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_NAME,
                      NULL,
                      &pRepo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_BASEURL,
                      NULL,
                      &pRepo->pszBaseUrl);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_METALINK,
                      NULL,
                      &pRepo->pszMetaLink);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_SKIP,
                      1,
                      &pRepo->nSkipIfUnavailable);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_GPGCHECK,
                      1,
                      &pRepo->nGPGCheck);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueStringArray(
                      pSections,
                      TDNF_REPO_KEY_GPGKEY,
                      &pRepo->ppszUrlGPGKeys);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_USERNAME,
                      NULL,
                      &pRepo->pszUser);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_PASSWORD,
                      NULL,
                      &pRepo->pszPass);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_PRIORITY,
                      50,
                      &pRepo->nPriority);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_TIMEOUT,
                      0,
                      &pRepo->nTimeout);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_RETRIES,
                      10,
                      &pRepo->nRetries);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_MINRATE,
                      0,
                      &pRepo->nMinrate);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_THROTTLE,
                      0,
                      &pRepo->nThrottle);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_SSL_VERIFY,
                      1,
                      &pRepo->nSSLVerify);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_SSL_CA_CERT,
                      NULL,
                      &pRepo->pszSSLCaCert);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_SSL_CLI_CERT,
                      NULL,
                      &pRepo->pszSSLClientCert);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_SSL_CLI_KEY,
                      NULL,
                      &pRepo->pszSSLClientKey);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_METADATA_EXPIRE,
                      TDNF_REPO_DEFAULT_METADATA_EXPIRE,
                      &pszMetadataExpire);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFParseMetadataExpire(
                      pszMetadataExpire,
                      &pRepo->lMetadataExpire);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszMetadataExpire);
        pszMetadataExpire = NULL;

        /* plugin event repo readconfig end */
        dwError = TDNFEventRepoReadConfigEnd(pTdnf, pSections);
        BAIL_ON_TDNF_ERROR(dwError);

        pRepo->pNext = pRepos;
        pRepos = pRepo;
        pRepo = NULL;
    }

    *ppRepos = pRepos;
cleanup:
    if(pData)
    {
        TDNFFreeConfigData(pData);
    }
    TDNF_SAFE_FREE_MEMORY(pszMetadataExpire);
    return dwError;

error:
    if(ppRepos)
    {
        *ppRepos = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pRepo);
    if(pRepos)
    {
        TDNFFreeReposInternal(pRepos);
    }
    goto cleanup;
}

uint32_t
TDNFRepoListFinalize(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;

    if(!pTdnf || !pTdnf->pArgs || !pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //There could be overrides to enable/disable
    //repo such as cmdline args, api overrides
    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_ENABLEREPO ||
           pSetOpt->nType == CMDOPT_DISABLEREPO)
        {
            dwError = TDNFAlterRepoState(
                          pTdnf->pRepos,
                          pSetOpt->nType == CMDOPT_ENABLEREPO,
                          pSetOpt->pszOptValue);
            BAIL_ON_TDNF_ERROR(dwError);
         }
         pSetOpt = pSetOpt->pNext;
    }

    //Now that the overrides are applied, replace config vars
    //for the repos that are enabled.
    pRepo = pTdnf->pRepos;
    while(pRepo)
    {
        if(pRepo->nEnabled)
        {
            if(pRepo->pszName)
            {
                dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszName);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            if(pRepo->pszBaseUrl)
            {
                dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszBaseUrl);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            if(pRepo->pszMetaLink)
            {
                dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszMetaLink);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        pRepo = pRepo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFAlterRepoState(
    PTDNF_REPO_DATA_INTERNAL pRepos,
    int nEnable,
    const char* pszId
    )
{
    uint32_t dwError = 0;
    int nMatch = 0;
    int nIsGlob = 0;
    if(!pRepos && IsNullOrEmptyString(pszId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nIsGlob = TDNFIsGlob(pszId);

    while(pRepos)
    {
        nMatch = 0;
        if(nIsGlob)
        {
            if(!fnmatch(pszId, pRepos->pszId, 0))
            {
                nMatch = 1;
            }
        }
        else if(!strcmp(pRepos->pszId, pszId))
        {
            nMatch = 1;
        }
        if(nMatch)
        {
            pRepos->nEnabled = nEnable;
            if(!nIsGlob)
            {
                break;
            }
        }
        pRepos = pRepos->pNext;
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCloneRepo(
    PTDNF_REPO_DATA_INTERNAL pRepoIn,
    PTDNF_REPO_DATA* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;

    if(!pRepoIn || !ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_REPO_DATA), (void**)&pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepo->nEnabled = pRepoIn->nEnabled;

    dwError = TDNFSafeAllocateString(pRepoIn->pszId, &pRepo->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pRepoIn->pszName, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pRepoIn->pszBaseUrl, &pRepo->pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(
                  pRepoIn->pszMetaLink,
                  &pRepo->pszMetaLink);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pRepoIn->ppszUrlGPGKeys) {
        dwError = TDNFAllocateStringArray(
                      pRepoIn->ppszUrlGPGKeys,
                      &pRepo->ppszUrlGPGKeys);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppRepo = pRepo;

cleanup:
    return dwError;

error:
    if(ppRepo)
    {
        *ppRepo = NULL;
    }
    if(pRepo)
    {
        TDNFFreeRepos(pRepo);
    }
    goto cleanup;
}

void
TDNFFreeReposInternal(
    PTDNF_REPO_DATA_INTERNAL pRepos
    )
{
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;
    while(pRepos)
    {
        pRepo = pRepos;
        TDNF_SAFE_FREE_MEMORY(pRepo->pszId);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszName);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszBaseUrl);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszMetaLink);
        TDNF_SAFE_FREE_STRINGARRAY(pRepo->ppszUrlGPGKeys);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszUser);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszPass);
        pRepos = pRepo->pNext;
        TDNF_SAFE_FREE_MEMORY(pRepo);
    }
}
