/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
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
    PTDNF_CMD_OPT pSetOpt = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    int nLen = 0;
    int nLenRepoExt = 0;
    char **ppszUrlIdTuple = NULL;

    if(!pTdnf || !pTdnf->pConf || !pTdnf->pArgs || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pConf = pTdnf->pConf;

    dwError = TDNFCreateCmdLineRepo(&pReposAll);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pSetOpt = pTdnf->pArgs->pSetOpt;
        pSetOpt;
        pSetOpt = pSetOpt->pNext)
    {
        if(strcmp(pSetOpt->pszOptName, "repofrompath") == 0)
        {
            TDNFSplitStringToArray(pSetOpt->pszOptValue, ",", &ppszUrlIdTuple);
            if ((ppszUrlIdTuple[0] == NULL) || ppszUrlIdTuple[1] == NULL)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            dwError = TDNFCreateRepoFromPath(&pReposAll,
                                             ppszUrlIdTuple[0],
                                             ppszUrlIdTuple[1]);
            BAIL_ON_TDNF_ERROR(dwError);

            TDNF_SAFE_FREE_STRINGARRAY(ppszUrlIdTuple);
            ppszUrlIdTuple = NULL;
        }
    }

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

        dwError = TDNFJoinPath(
                      &pszRepoFilePath,
                      pConf->pszRepoDir,
                      pEnt->d_name,
                      NULL);
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
    TDNF_SAFE_FREE_STRINGARRAY(ppszUrlIdTuple);

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

    dwError = TDNFCreateRepo(&pRepo, CMDLINE_REPO_NAME);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(CMDLINE_REPO_NAME, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppRepo = pRepo;
cleanup:
    return dwError;
error:
    if(pRepo)
    {
        TDNFFreeReposInternal(pRepo);
    }
    goto cleanup;
}

uint32_t
TDNFCreateRepoFromPath(
    PTDNF_REPO_DATA_INTERNAL* ppRepo,
    const char *pszId,
    const char *pszPath
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;
    int nIsDir = 0;
    int nDummy = 0;

    if(!ppRepo || !pszId || !pszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCreateRepo(&pRepo, pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    /* we want it enabled, or there was no point in adding it */
    pRepo->nEnabled = 1;

    dwError = TDNFSafeAllocateString(pszId, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    /* '/some/dir' => 'file:///some/dir */
    if (pszPath[0] == '/')
    {
        dwError = TDNFIsDir(pszPath, &nIsDir);
        BAIL_ON_TDNF_ERROR(dwError);

        if (nIsDir)
        {
            dwError = TDNFAllocateStringPrintf(&pRepo->pszBaseUrl, "file://%s", pszPath);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
        /* valid prefixes including file:// will not return an error */
        dwError = TDNFUriIsRemote(pszPath, &nDummy);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(pszPath, &pRepo->pszBaseUrl);
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
        TDNFFreeReposInternal(pRepo);
    }
    goto cleanup;
}

uint32_t
TDNFCreateRepo(
    PTDNF_REPO_DATA_INTERNAL* ppRepo,
    const char *pszId
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;

    if(!ppRepo || !pszId)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_DATA_INTERNAL),
                  (void**)&pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pszId, &pRepo->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepo->nEnabled = TDNF_REPO_DEFAULT_ENABLED;
    pRepo->nSkipIfUnavailable = TDNF_REPO_DEFAULT_SKIP;
    pRepo->nGPGCheck = TDNF_REPO_DEFAULT_GPGCHECK;
    pRepo->nSSLVerify = TDNF_REPO_DEFAULT_SSLVERIFY;
    pRepo->lMetadataExpire = TDNF_REPO_DEFAULT_METADATA_EXPIRE;
    pRepo->nPriority = TDNF_REPO_DEFAULT_PRIORITY;
    pRepo->nTimeout = TDNF_REPO_DEFAULT_TIMEOUT;
    pRepo->nMinrate = TDNF_REPO_DEFAULT_MINRATE;
    pRepo->nThrottle = TDNF_REPO_DEFAULT_THROTTLE;
    pRepo->nRetries = TDNF_REPO_DEFAULT_RETRIES;

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
                      TDNF_REPO_DEFAULT_ENABLED,
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
                      TDNF_REPO_DEFAULT_SKIP,
                      &pRepo->nSkipIfUnavailable);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_GPGCHECK,
                      TDNF_REPO_DEFAULT_GPGCHECK,
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
                      TDNF_REPO_DEFAULT_PRIORITY,
                      &pRepo->nPriority);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_TIMEOUT,
                      TDNF_REPO_DEFAULT_TIMEOUT,
                      &pRepo->nTimeout);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_RETRIES,
                      TDNF_REPO_DEFAULT_RETRIES,
                      &pRepo->nRetries);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_MINRATE,
                      TDNF_REPO_DEFAULT_MINRATE,
                      &pRepo->nMinrate);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueInt(
                      pSections,
                      TDNF_REPO_KEY_THROTTLE,
                      TDNF_REPO_DEFAULT_THROTTLE,
                      &pRepo->nThrottle);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValueBoolean(
                      pSections,
                      TDNF_REPO_KEY_SSL_VERIFY,
                      TDNF_REPO_DEFAULT_SSLVERIFY,
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
                      TDNF_REPO_DEFAULT_METADATA_EXPIRE_STR,
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
    int nRepoidSeen = 0;

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
        else if(strcmp(pSetOpt->pszOptName, "repoid") == 0)
        {
            if (!nRepoidSeen)
            {
                dwError = TDNFAlterRepoState(
                              pTdnf->pRepos, 0, "*");
                BAIL_ON_TDNF_ERROR(dwError);
                nRepoidSeen = 1;
            }
            dwError = TDNFAlterRepoState(
                          pTdnf->pRepos,
                          1,
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
