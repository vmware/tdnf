/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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
    PTDNF_REPO_DATA* ppReposAll
    )
{
    uint32_t dwError = 0;
    char* pszRepoFilePath = NULL;
    PTDNF_REPO_DATA pReposAll = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL;
    PTDNF_REPO_DATA pRepos = NULL;
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

    pDir = opendir(pConf->pszRepoDir);
    if(pDir == NULL)
    {
        dwError = ERROR_TDNF_REPO_DIR_OPEN;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
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
            TDNFFreeRepos(pRepos);
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
        TDNFFreeRepos(pReposAll);
    }
    goto cleanup;}

uint32_t
TDNFLoadReposFromFile(
    PTDNF pTdnf,
    char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    )
{
    uint32_t dwError = 0;

    char* pszRepo = NULL;
    char* pszMetadataExpire = NULL;

    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

    PCONF_DATA pData = NULL;
    PCONF_SECTION pSections = NULL;

    dwError = TDNFReadConfigFile(pszRepoFile, 0, &pData);
    BAIL_ON_TDNF_ERROR(dwError);
    pSections = pData->pSections;
    for(; pSections; pSections = pSections->pNext)
    {
        pszRepo = pSections->pszName;

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_REPO_DATA),
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

        dwError = TDNFReadKeyValue(
                      pSections,
                      TDNF_REPO_KEY_GPGKEY,
                      NULL,
                      &pRepo->pszUrlGPGKey);
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
        TDNFFreeRepos(pRepos);
    }
    goto cleanup;}

uint32_t
TDNFRepoListFinalize(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

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
    PTDNF_REPO_DATA pRepos,
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
    PTDNF_REPO_DATA pRepoIn,
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

    dwError = TDNFSafeAllocateString(
                  pRepoIn->pszUrlGPGKey,
                  &pRepo->pszUrlGPGKey);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pRepoIn->pszUser, &pRepo->pszUser);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pRepoIn->pszPass, &pRepo->pszPass);
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
        TDNFFreeRepos(pRepo);
    }
    goto cleanup;
}

void
TDNFFreeRepos(
    PTDNF_REPO_DATA pRepos
    )
{
    PTDNF_REPO_DATA pRepo = NULL;
    while(pRepos)
    {
        pRepo = pRepos;
        TDNF_SAFE_FREE_MEMORY(pRepo->pszId);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszName);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszBaseUrl);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszMetaLink);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszUrlGPGKey);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszUser);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszPass);
        pRepos = pRepo->pNext;
        TDNF_SAFE_FREE_MEMORY(pRepo);
    }
}