/*
 * Copyright (C) 2022-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

#include "../../llconf/nodes.h"

static
uint32_t
TDNFHasRepo(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    int *pnHasRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_METALINK_DATA pData = NULL;
    int nHasRepo = 0;

    if (!pHandle || IsNullOrEmptyString(pcszRepoId) || !pnHasRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pData = pHandle->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pData->pszRepoId, pcszRepoId) == 0)
        {
            nHasRepo = 1;
            break;
        }
    }

    *pnHasRepo = nHasRepo;

error:
    return dwError;
}

uint32_t
TDNFMetalinkReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    char *pszMetalink = NULL;
    struct cnfnode *cn_section = NULL, *cn;
    PTDNF_METALINK_DATA pData = NULL;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* event has a repo section which has all the config data */
    dwError = TDNFEventContextGetItemPtr(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_SECTION,
                  (const void **)&cn_section);
    BAIL_ON_TDNF_ERROR(dwError);

    for(cn = cn_section->first_child; cn; cn = cn->next)
    {
        if ((cn->name[0] == '.') || (cn->value == NULL))
            continue;

        if (strcmp(cn->name, TDNF_REPO_CONFIG_METALINK_KEY) == 0)
        {
            if (pszMetalink != NULL) free(pszMetalink);
            pszMetalink = strdup(cn->value);
        }
    }

    /*
     * if metalink is set, keep this repo id
     * section name is the repo id
    */
    if (pszMetalink)
    {
        dwError = TDNFAllocateMemory(sizeof(*pData), 1, (void **)&pData);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(cn_section->name, &pData->pszRepoId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pszMetalink, &pData->pszMetalink);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFConfigReplaceVars(pHandle->pTdnf, &pData->pszMetalink);
        BAIL_ON_TDNF_ERROR(dwError);

        pData->pNext = pHandle->pData;
        pHandle->pData = pData;
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszMetalink);
    return dwError;

error:
    TDNFFreeMetalinkData(pData);
    goto cleanup;
}

static
uint32_t
TDNFParseAndGetURLFromMetalink(
    PTDNF pTdnf,
    const char *pszFile,
    TDNF_ML_CTX *ml_ctx
    )
{
    FILE* file = NULL;
    uint32_t dwError = 0;

    if (!pTdnf ||
       IsNullOrEmptyString(pszFile) ||
       !ml_ctx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    file = fopen(pszFile, "r");
    if (file == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    dwError = TDNFMetalinkParseFile(ml_ctx, file, TDNF_REPO_METADATA_FILE_NAME);
    if (dwError)
    {
        pr_err("Unable to parse metalink, ERROR: code=%d\n", dwError);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //sort the URL's in List based on preference.
    TDNFSortListOnPreference(&ml_ctx->urls);

cleanup:
    if (file != NULL)
    {
        fclose(file);
    }
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
TDNFGetUrlsFromMLCtx(
    PTDNF pTdnf,
    TDNF_ML_CTX *ml_ctx,
    char ***pppszBaseUrls
    )
{
    uint32_t dwError = 0;
    TDNF_ML_URL_LIST *urlList = NULL;
    TDNF_ML_URL_INFO *urlInfo = NULL;
    char **ppszBaseUrls = NULL;
    char buf[BUFSIZ] = {0};
    int i, count = 0;

    if (!pTdnf || !ml_ctx || !pppszBaseUrls)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (urlList = ml_ctx->urls; urlList; urlList = urlList->next) {
        count++;
    }

    dwError = TDNFAllocateMemory(sizeof(char **), count+1, (void **)&ppszBaseUrls);
    BAIL_ON_TDNF_ERROR(dwError);

    for (urlList = ml_ctx->urls, i = 0; urlList; urlList = urlList->next, i++) {
        urlInfo = urlList->data;
        if (urlInfo == NULL)
        {
            dwError = ERROR_TDNF_INVALID_REPO_FILE;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFStringEndsWith(urlInfo->url, TDNF_REPO_METADATA_FILE_PATH);
        if (dwError)
        {
            dwError = ERROR_TDNF_INVALID_REPO_FILE;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        strncpy(buf, urlInfo->url, BUFSIZ-1);
        buf[BUFSIZ-1] = '\0'; // force terminate
        dwError = TDNFTrimSuffix(buf, TDNF_REPO_METADATA_FILE_PATH);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(buf, &ppszBaseUrls[i]);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *pppszBaseUrls = ppszBaseUrls;
cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_STRINGARRAY(ppszBaseUrls);
    goto cleanup;
}

static
uint32_t
TDNFMetalinkGetBaseURLs(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    const char *pcszRepoDataDir
)
{
    uint32_t dwError = 0;
    PTDNF pTdnf;
    PTDNF_REPO_DATA pRepo = NULL;
    PTDNF_METALINK_DATA pData = NULL;
    const char *pszMetalink = NULL;
    char *pszMetaLinkFile = NULL;
    TDNF_ML_CTX *ml_ctx = NULL;

    if (!pHandle || !pHandle->pTdnf || IsNullOrEmptyString(pcszRepoId) ||
        IsNullOrEmptyString(pcszRepoDataDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pTdnf = pHandle->pTdnf;

    dwError = TDNFFindRepoById(pTdnf, pcszRepoId, &pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pData = pHandle->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pData->pszRepoId, pcszRepoId) == 0)
        {
            pszMetalink = pData->pszMetalink;
            break;
        }
    }
    if (pszMetalink == NULL) {
        /* shouldn't happen - we checked for this in
           TDNFMetalinkReadConfig() */
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszMetaLinkFile,
                           pcszRepoDataDir,
                           TDNF_REPO_METALINK_FILE_NAME,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pTdnf->pArgs->nRefresh || access(pszMetaLinkFile, F_OK))
    {
        dwError = TDNFUtilsMakeDirs(pcszRepoDataDir);
        if (dwError == ERROR_TDNF_ALREADY_EXISTS)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadFile(pTdnf, pRepo, pRepo->pszMetaLink,
                                   pszMetaLinkFile, pRepo->pszId);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_ML_CTX), (void **)&ml_ctx);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFParseAndGetURLFromMetalink(pTdnf,
                pszMetaLinkFile, ml_ctx);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetUrlsFromMLCtx(pTdnf, ml_ctx, &pRepo->ppszBaseUrls);
    BAIL_ON_TDNF_ERROR(dwError);

    pData->ml_ctx = ml_ctx;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszMetaLinkFile);
    return dwError;
error:
    pr_err("Error: %s %u\n", __FUNCTION__, dwError);
    goto cleanup;
}

uint32_t
TDNFMetalinkRepoMDDownloadStart(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    const char *pcszRepoId = NULL;
    const char *pcszRepoDataDir = NULL;
    int nHasRepo = 0;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* we are looking for repo id first */
    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_ID,
                  (const char **)&pcszRepoId);
    BAIL_ON_TDNF_ERROR(dwError);

    /* check if this repo id is in list for repo_gpgcheck */
    dwError = TDNFHasRepo(pHandle, pcszRepoId, &nHasRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    /* if repo is not in list, return immediately */
    if (nHasRepo == 0)
    {
        goto cleanup;
    }

    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_DATADIR,
                  (const char **)&pcszRepoDataDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFMetalinkGetBaseURLs(pHandle, pcszRepoId, pcszRepoDataDir);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
TDNFCheckRepoMDFileHash(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    const char *pcszRepoMDFile
)
{
    uint32_t dwError = 0;
    PTDNF pTdnf;
    PTDNF_REPO_DATA pRepo = NULL;
    PTDNF_METALINK_DATA pData = NULL;
    TDNF_ML_CTX *ml_ctx = NULL;

    if (!pHandle || !pHandle->pTdnf || IsNullOrEmptyString(pcszRepoId) ||
        IsNullOrEmptyString(pcszRepoMDFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pTdnf = pHandle->pTdnf;

    dwError = TDNFFindRepoById(pTdnf, pcszRepoId, &pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pData = pHandle->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pData->pszRepoId, pcszRepoId) == 0)
        {
            ml_ctx = pData->ml_ctx;
            break;
        }
    }
    if (ml_ctx == NULL) {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCheckRepoMDFileHashFromMetalink(pcszRepoMDFile, ml_ctx);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    pr_err("Error: %s %u\n", __FUNCTION__, dwError);
    goto cleanup;
}

uint32_t
TDNFMetalinkRepoMDDownloadEnd(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    const char *pcszRepoId = NULL;
    const char *pcszRepoMDFile = NULL;
    int nHasRepo = 0;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* we are looking for repo id first */
    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_ID,
                  (const char **)&pcszRepoId);
    BAIL_ON_TDNF_ERROR(dwError);

    /* check if this repo id is in list for metalink */
    dwError = TDNFHasRepo(pHandle, pcszRepoId, &nHasRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    /* if repo is not in list, return immediately */
    if (nHasRepo == 0)
    {
        goto cleanup;
    }

    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_MD_FILE,
                  (const char **)&pcszRepoMDFile);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCheckRepoMDFileHash(pHandle, pcszRepoId, pcszRepoMDFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}
