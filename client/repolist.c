/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

#include "../llconf/nodes.h"
#include "../llconf/modules.h"
#include "../llconf/entry.h"
#include "../llconf/ini.h"


static
uint32_t
TDNFCreateCmdLineRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo
    );

static
uint32_t
TDNFCreateRepoFromPath(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pzsId,
    const char *pszPath
    );

static
uint32_t
TDNFCreateRepoFromDirectory(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pzsId,
    const char *pszPath
    );

static
uint32_t
TDNFCreateRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pszId
    );



uint32_t
TDNFLoadRepoData(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppReposAll
    )
{
    uint32_t dwError = 0;
    char* pszRepoFilePath = NULL;
    PTDNF_REPO_DATA pReposAll = NULL;
    PTDNF_REPO_DATA *ppRepoNext = NULL;
    PTDNF_CONF pConf = NULL;
    PTDNF_CMD_OPT pSetOpt = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    char **ppszUrlIdTuple = NULL;
    PTDNF_REPO_DATA pRepoParsePre = NULL;
    PTDNF_REPO_DATA pRepoParseNext = NULL;

    if(!pTdnf || !pTdnf->pConf || !pTdnf->pArgs || !ppReposAll)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pConf = pTdnf->pConf;

    ppRepoNext = &pReposAll;

    dwError = TDNFCreateCmdLineRepo(pTdnf, ppRepoNext);
    BAIL_ON_TDNF_ERROR(dwError);

    ppRepoNext = &((*ppRepoNext)->pNext);

    for(pSetOpt = pTdnf->pArgs->pSetOpt;
        pSetOpt;
        pSetOpt = pSetOpt->pNext)
    {
        if(strcmp(pSetOpt->pszOptName, "repofrompath") == 0)
        {
            dwError = TDNFSplitStringToArray(pSetOpt->pszOptValue, ",", &ppszUrlIdTuple);
            BAIL_ON_TDNF_ERROR(dwError);
            if ((ppszUrlIdTuple[0] == NULL) || ppszUrlIdTuple[1] == NULL)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFCreateRepoFromPath(pTdnf, ppRepoNext,
                                             ppszUrlIdTuple[0],
                                             ppszUrlIdTuple[1]);
            BAIL_ON_TDNF_ERROR(dwError);

            ppRepoNext = &((*ppRepoNext)->pNext);

            TDNF_SAFE_FREE_STRINGARRAY(ppszUrlIdTuple);
            ppszUrlIdTuple = NULL;
        }
        else if(strcmp(pSetOpt->pszOptName, "repofromdir") == 0)
        {
            dwError = TDNFSplitStringToArray(pSetOpt->pszOptValue, ",", &ppszUrlIdTuple);
            BAIL_ON_TDNF_ERROR(dwError);
            if ((ppszUrlIdTuple[0] == NULL) || ppszUrlIdTuple[1] == NULL)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            dwError = TDNFCreateRepoFromDirectory(pTdnf, &pReposAll,
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
        int nLen = strlen(pEnt->d_name);
        int nLenRepoExt = strlen(TDNF_REPO_EXT);
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

        dwError = TDNFLoadReposFromFile(pTdnf, pszRepoFilePath, ppRepoNext);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszRepoFilePath);
        pszRepoFilePath = NULL;
        /* may have added multiple repos, go to last one */
        while (*ppRepoNext)
            ppRepoNext = &((*ppRepoNext)->pNext);
    }

    for (pRepoParsePre = pReposAll; pRepoParsePre; pRepoParsePre = pRepoParsePre->pNext) {

        for (pRepoParseNext = pRepoParsePre->pNext; pRepoParseNext; pRepoParseNext = pRepoParseNext->pNext) {
            if (!strcmp(pRepoParsePre->pszId, pRepoParseNext->pszId)) {
                pr_err("ERROR: duplicate repo id: %s\n", pRepoParsePre->pszId);
                dwError = ERROR_TDNF_DUPLICATE_REPO_ID;
                BAIL_ON_TDNF_ERROR(dwError);
            }
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

static
uint32_t
TDNFCreateCmdLineRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo
    )
{
    uint32_t dwError;
    PTDNF_REPO_DATA pRepo = NULL;

    if(!ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCreateRepo(pTdnf, &pRepo, CMDLINE_REPO_NAME);
    BAIL_ON_TDNF_ERROR(dwError);
    pRepo->nHasMetaData = 0;

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

static
uint32_t
TDNFCreateRepoFromDirectory(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pszId,
    const char *pszPath
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    int nIsDir = 0;

    if(!ppRepo || !pszId || !pszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCreateRepo(pTdnf, &pRepo, pszId);
    BAIL_ON_TDNF_ERROR(dwError);
    pRepo->nHasMetaData = 0;

    /* we want it enabled, or there was no point in adding it */
    pRepo->nEnabled = 1;

    dwError = TDNFSafeAllocateString(pszId, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFIsDir(pszPath, &nIsDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!nIsDir)
    {
        pr_err("%s is not a directory\n", pszPath);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(sizeof(char **), 2, (void **)&pRepo->ppszBaseUrls);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pszPath, &pRepo->ppszBaseUrls[0]);
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

static
uint32_t
TDNFCreateRepoFromPath(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pszId,
    const char *pszPath
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    int nIsDir = 0;
    int nDummy = 0;

    if(!ppRepo || !pszId || !pszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCreateRepo(pTdnf, &pRepo, pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    /* we want it enabled, or there was no point in adding it */
    pRepo->nEnabled = 1;

    dwError = TDNFSafeAllocateString(pszId, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(sizeof(char **), 2, (void **)&pRepo->ppszBaseUrls);
    BAIL_ON_TDNF_ERROR(dwError);

    /* '/some/dir' => 'file:///some/dir */
    if (pszPath[0] == '/')
    {
        dwError = TDNFIsDir(pszPath, &nIsDir);
        BAIL_ON_TDNF_ERROR(dwError);

        if (nIsDir)
        {
            dwError = TDNFAllocateStringPrintf(&pRepo->ppszBaseUrls[0], "file://%s", pszPath);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
        /* valid prefixes including file:// will not return an error */
        dwError = TDNFUriIsRemote(pszPath, &nDummy);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFSafeAllocateString(pszPath, &pRepo->ppszBaseUrls[0]);
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

static
uint32_t
TDNFCreateRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppRepo,
    const char *pszId
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;

    if(!ppRepo || !pszId)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_DATA),
                  (void**)&pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pszId, &pRepo->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepo->nEnabled = TDNF_REPO_DEFAULT_ENABLED;
    pRepo->nHasMetaData = 1;
    pRepo->nSkipIfUnavailable = TDNF_REPO_DEFAULT_SKIP;
    pRepo->nGPGCheck = TDNF_REPO_DEFAULT_GPGCHECK;
    pRepo->nSSLVerify = pTdnf->pConf->nSSLVerify;
    pRepo->lMetadataExpire = TDNF_REPO_DEFAULT_METADATA_EXPIRE;
    pRepo->nPriority = TDNF_REPO_DEFAULT_PRIORITY;
    pRepo->nTimeout = TDNF_REPO_DEFAULT_TIMEOUT;
    pRepo->nMinrate = TDNF_REPO_DEFAULT_MINRATE;
    pRepo->nThrottle = TDNF_REPO_DEFAULT_THROTTLE;
    pRepo->nRetries = TDNF_REPO_DEFAULT_RETRIES;
    pRepo->nSkipMDFileLists = TDNF_REPO_DEFAULT_SKIP_MD_FILELISTS;
    pRepo->nSkipMDUpdateInfo = TDNF_REPO_DEFAULT_SKIP_MD_UPDATEINFO;
    pRepo->nSkipMDOther = TDNF_REPO_DEFAULT_SKIP_MD_OTHER;

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
    const struct cnfnode *cn_section
    )
{
    uint32_t dwError = 0;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf || !cn_section)
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
                  cn_section);
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
    const struct cnfnode *cn_section
    )
{
    uint32_t dwError = 0;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf || !cn_section)
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
                  cn_section);
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
    const char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    )
{
    uint32_t dwError = 0;
    char *pszMetadataExpire = NULL;

    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

    struct cnfnode *cn_conf = NULL, *cn_section, *cn;
    struct cnfmodule *mod_ini;

    mod_ini = find_cnfmodule("ini");
    if (mod_ini == NULL) {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    cn_conf = cnfmodule_parse_file(mod_ini, pszRepoFile);
    if (cn_conf == NULL)
    {
        if (errno != 0)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_TDNF_CONF_FILE_LOAD;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    /* cn_conf == NULL => we will not reach here */
    /* coverity[var_deref_op] */
    for(cn_section = cn_conf->first_child; cn_section; cn_section = cn_section->next)
    {
        if (cn_section->name[0] == '.')
            continue;

        dwError = TDNFCreateRepo(pTdnf, &pRepo, cn_section->name);
        BAIL_ON_TDNF_ERROR(dwError);

        /* plugin event repo readconfig start */
        dwError = TDNFEventRepoReadConfigStart(pTdnf, cn_section);
        BAIL_ON_TDNF_ERROR(dwError);

        for(cn = cn_section->first_child; cn; cn = cn->next)
        {
            if ((cn->name[0] == '.') || (cn->value == NULL))
                continue;

            if (strcmp(cn->name, TDNF_REPO_KEY_ENABLED) == 0)
            {
                pRepo->nEnabled = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_NAME) == 0)
            {
                pRepo->pszName = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_BASEURL) == 0)
            {
                dwError = TDNFSplitStringToArray(cn->value,
                                                 " ", &pRepo->ppszBaseUrls);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_METALINK) == 0)
            {
                pRepo->pszMetaLink = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_MIRRORLIST) == 0)
            {
                pRepo->pszMirrorList = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SKIP) == 0)
            {
                pRepo->nSkipIfUnavailable = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_GPGCHECK) == 0)
            {
                pRepo->nGPGCheck = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_GPGKEY) == 0)
            {
                dwError = TDNFSplitStringToArray(cn->value,
                                                 " ", &pRepo->ppszUrlGPGKeys);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_USERNAME) == 0)
            {
                pRepo->pszUser = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_PASSWORD) == 0)
            {
                pRepo->pszPass = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_PRIORITY) == 0)
            {
                pRepo->nPriority = strtoi(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_TIMEOUT) == 0)
            {
                pRepo->nTimeout = strtoi(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_RETRIES) == 0)
            {
                pRepo->nRetries = strtoi(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_MINRATE) == 0)
            {
                pRepo->nMinrate = strtoi(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_THROTTLE) == 0)
            {
                pRepo->nThrottle = strtoi(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SSL_VERIFY) == 0)
            {
                pRepo->nSSLVerify = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SSL_CA_CERT) == 0)
            {
                pRepo->pszSSLCaCert = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SSL_CLI_CERT) == 0)
            {
                pRepo->pszSSLClientCert = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SSL_CLI_KEY) == 0)
            {
                pRepo->pszSSLClientKey = strdup(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_METADATA_EXPIRE) == 0)
            {
                dwError = TDNFParseMetadataExpire(
                              cn->value,
                              &pRepo->lMetadataExpire);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SKIP_MD_FILELISTS) == 0)
            {
                pRepo->nSkipMDFileLists = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SKIP_MD_UPDATEINFO) == 0)
            {
                pRepo->nSkipMDUpdateInfo = isTrue(cn->value);
            }
            else if (strcmp(cn->name, TDNF_REPO_KEY_SKIP_MD_OTHER) == 0)
            {
                pRepo->nSkipMDOther = isTrue(cn->value);
            }
        }
        /* plugin event repo readconfig end */
        dwError = TDNFEventRepoReadConfigEnd(pTdnf, cn_section);
        BAIL_ON_TDNF_ERROR(dwError);

        /* default to repo id if name isn't set */
        if (pRepo->pszName == NULL)
            pRepo->pszName = strdup(pRepo->pszId);

        pRepo->pNext = pRepos;
        pRepos = pRepo;
        pRepo = NULL;
    }

    *ppRepos = pRepos;

cleanup:
    destroy_cnftree(cn_conf);
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
    PTDNF_REPO_DATA pRepo = NULL;
    int nRepoidSeen = 0;

    if(!pTdnf || !pTdnf->pArgs || !pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* There could be overrides to enable/disable
       repo such as cmdline args, api overrides */
    for (pSetOpt = pTdnf->pArgs->pSetOpt; pSetOpt; pSetOpt = pSetOpt->pNext)
    {
        if(strcmp(pSetOpt->pszOptName, "enablerepo") == 0)
        {
            dwError = TDNFAlterRepoState(
                          pTdnf->pRepos,
                          1,
                          pSetOpt->pszOptValue);
        }
        else if(strcmp(pSetOpt->pszOptName, "disablerepo") == 0)
        {
            dwError = TDNFAlterRepoState(
                          pTdnf->pRepos,
                          0,
                          pSetOpt->pszOptValue);
        }
        else if((strcmp(pSetOpt->pszOptName, "repo") == 0) ||
                (strcmp(pSetOpt->pszOptName, "repoid") == 0))
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
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* Now that the overrides are applied, replace config vars
       for all repos. */
    for(pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
    {
        if(pRepo->pszName)
        {
            dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszName);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(pRepo->ppszBaseUrls)
        {
            for (int i = 0; pRepo->ppszBaseUrls[i]; i++)
            {
                dwError = TDNFConfigReplaceVars(pTdnf, &(pRepo->ppszBaseUrls[i]));
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        if(pRepo->pszMetaLink)
        {
            dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszMetaLink);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(pRepo->pszMirrorList)
        {
            dwError = TDNFConfigReplaceVars(pTdnf, &pRepo->pszMirrorList);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (pRepo->ppszUrlGPGKeys)
        {
            for (int i = 0; pRepo->ppszUrlGPGKeys[i]; i++)
            {
                dwError = TDNFConfigReplaceVars(pTdnf, &(pRepo->ppszUrlGPGKeys[i]));
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }

        if (pRepo->pszMetaLink)
        {
            dwError = SolvCreateRepoCacheName(pRepo->pszId,
                                              pRepo->pszMetaLink,
                                              &pRepo->pszCacheName);
        }
        else if (pRepo->pszMirrorList)
        {
            dwError = SolvCreateRepoCacheName(pRepo->pszId,
                                              pRepo->pszMirrorList,
                                              &pRepo->pszCacheName);
        }
        else if (pRepo->ppszBaseUrls && pRepo->ppszBaseUrls[0])
        {
            dwError = SolvCreateRepoCacheName(pRepo->pszId,
                                              pRepo->ppszBaseUrls[0],
                                              &pRepo->pszCacheName);
        }
        BAIL_ON_TDNF_ERROR(dwError);
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
    int nIsGlob = 0;
    if(!pRepos && IsNullOrEmptyString(pszId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nIsGlob = TDNFIsGlob(pszId);

    for (int nMatch = 0; pRepos; pRepos = pRepos->pNext)
    {
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

    /* cli needs just nEnabled, pszId, pszName */
    pRepo->nEnabled = pRepoIn->nEnabled;

    dwError = TDNFSafeAllocateString(pRepoIn->pszId, &pRepo->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSafeAllocateString(pRepoIn->pszName, &pRepo->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    /* python needs also pszBaseUrl and pszMetaLink */
    if (pRepoIn->ppszBaseUrls && pRepoIn->ppszBaseUrls[0]) {
        dwError = TDNFAllocateStringArray(pRepoIn->ppszBaseUrls, &pRepo->ppszBaseUrls);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFSafeAllocateString(
                  pRepoIn->pszMetaLink,
                  &pRepo->pszMetaLink);
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
TDNFFreeReposInternal(
    PTDNF_REPO_DATA pRepos
    )
{
    PTDNF_REPO_DATA pRepo = NULL;
    while(pRepos)
    {
        pRepo = pRepos;
        TDNF_SAFE_FREE_MEMORY(pRepo->pszId);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszName);
        TDNF_SAFE_FREE_STRINGARRAY(pRepo->ppszBaseUrls);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszMetaLink);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszMirrorList);
        TDNF_SAFE_FREE_STRINGARRAY(pRepo->ppszUrlGPGKeys);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszUser);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszPass);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszCacheName);
        pRepos = pRepo->pNext;
        TDNF_SAFE_FREE_MEMORY(pRepo);
    }
}
