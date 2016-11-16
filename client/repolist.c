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
    const char* pszFile = NULL;
    
    PTDNF_REPO_DATA pReposAll = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL; 
    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_CONF pConf = NULL;

    GDir* pDir = NULL;

    if(!pTdnf || !pTdnf->pConf || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pConf = pTdnf->pConf;

    pDir = g_dir_open(pConf->pszRepoDir, 0, NULL);
    if(!pDir)
    {
        dwError = ERROR_TDNF_REPO_DIR_OPEN;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((pszFile = g_dir_read_name (pDir)) != NULL)
    {
        if (!g_str_has_suffix (pszFile, TDNF_REPO_EXT))
        {
            continue;
        }
        pszRepoFilePath = g_build_filename(pConf->pszRepoDir, pszFile, NULL);

        dwError = TDNFLoadReposFromFile(pTdnf, pszRepoFilePath, &pRepos);
        BAIL_ON_TDNF_ERROR(dwError);

        g_free(pszRepoFilePath);
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
        g_dir_close(pDir);
    }
    if(pszRepoFilePath)
    {
        g_free(pszRepoFilePath);
    }
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
    goto cleanup;
}

uint32_t
TDNFLoadReposFromFile(
    PTDNF pTdnf,
    char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    )
{
    uint32_t dwError = 0;
    int i = 0;

    GKeyFile* pKeyFile = NULL;
    char** ppszRepos = NULL;
    char* pszRepo = NULL;
    char* pszValue = NULL;
    char* pszMetadataExpire = NULL;

    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

    pKeyFile = g_key_file_new(); 
    if(!pKeyFile)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!g_key_file_load_from_file(
            pKeyFile,
            pszRepoFile,
            G_KEY_FILE_KEEP_COMMENTS,
            NULL))
    {
        dwError = ERROR_TDNF_REPO_FILE_LOAD;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    ppszRepos = g_key_file_get_groups (pKeyFile, NULL);
    if(!ppszRepos)
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(ppszRepos[i])
    {
        pszRepo = ppszRepos[i];

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_REPO_DATA),
                      (void**)&pRepo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pszRepo, &pRepo->pszId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValueBoolean(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_ENABLED,
                      0,
                      &pRepo->nEnabled);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_NAME,
                      NULL,
                      &pRepo->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_BASEURL,
                      NULL,
                      &pRepo->pszBaseUrl);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_METALINK,
                      NULL,
                      &pRepo->pszMetaLink);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValueBoolean(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_SKIP,
                      1,
                      &pRepo->nSkipIfUnavailable);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValueBoolean(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_GPGCHECK,
                      1,
                      &pRepo->nGPGCheck);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_GPGKEY,
                      NULL,
                      &pRepo->pszUrlGPGKey);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_USERNAME,
                      NULL,
                      &pRepo->pszUser);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_PASSWORD,
                      NULL,
                      &pRepo->pszPass);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFRepoGetKeyValue(
                      pKeyFile,
                      pszRepo,
                      TDNF_REPO_KEY_METADATA_EXPIRE,
                      NULL,
                      &pszMetadataExpire);
        BAIL_ON_TDNF_ERROR(dwError);

        //Set
        if(IsNullOrEmptyString(pszMetadataExpire))
        {
            TDNF_SAFE_FREE_MEMORY(pszMetadataExpire);
            dwError = TDNFAllocateString(
                          TDNF_REPO_DEFAULT_METADATA_EXPIRE,
                          &pszMetadataExpire);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFParseMetadataExpire(
                      pszMetadataExpire,
                      &pRepo->lMetadataExpire);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszMetadataExpire);
        pszMetadataExpire = NULL;

        pRepo->pNext = pRepos;
        pRepos = pRepo;
        pRepo = NULL;
        i++; 
    }

    *ppRepos = pRepos;
cleanup:
    if(pKeyFile)
    {
        g_key_file_free(pKeyFile);
    }
    g_free(pszValue);
    g_strfreev(ppszRepos);
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
    goto cleanup;
}

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