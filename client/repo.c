/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : repo.c
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

//Download repo metadata and initialize
uint32_t
TDNFInitRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    PSolvSack pSack
    )
{
    uint32_t dwError = 0;
    char* pszRepoDataDir = NULL;
    char* pszRepoCacheDir = NULL;
    PTDNF_REPO_METADATA pRepoMD = NULL;
    Repo* pRepo = NULL;
    Pool* pPool = NULL;
    int nUseMetaDataCache = 0;
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo = NULL;

    if (!pTdnf || !pRepoData || !pSack || !pSack->pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPool = pSack->pPool;

    dwError = TDNFGetCachePath(pTdnf, pRepoData,
                               NULL, NULL,
                               &pszRepoCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(
                  &pszRepoDataDir,
                  pszRepoCacheDir,
                  TDNF_REPODATA_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetRepoMD(pTdnf,
                            pRepoData,
                            pszRepoDataDir,
                            &pRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(SOLV_REPO_INFO_INTERNAL),
                  (void**)&pSolvRepoInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepo = repo_create(pPool, pRepoData->pszId);

    if (!pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pSolvRepoInfo->pRepo = pRepo;
    pSolvRepoInfo->pszRepoCacheDir = pszRepoCacheDir;
    pRepo->appdata = pSolvRepoInfo;

    dwError = SolvCalculateCookieForFile(pRepoMD->pszRepoMD, pSolvRepoInfo->cookie);
    BAIL_ON_TDNF_ERROR(dwError);

    pSolvRepoInfo->nCookieSet = 1;
    dwError = SolvUseMetaDataCache(pSack, pSolvRepoInfo, &nUseMetaDataCache);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nUseMetaDataCache == 0)
    {
        dwError = TDNFInitRepoFromMetadata(pSolvRepoInfo->pRepo, pRepoData->pszId, pRepoMD);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvCreateMetaDataCache(pSack, pSolvRepoInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pool_createwhatprovides(pPool);

cleanup:
    TDNFFreeRepoMetadata(pRepoMD);
    TDNF_SAFE_FREE_MEMORY(pszRepoDataDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    TDNF_SAFE_FREE_MEMORY(pSolvRepoInfo);
    return dwError;
error:
    if(pRepo)
    {
        repo_free(pRepo, 1);
    }
    //If there is an error during init, log the error
    //remove any cache data that could be potentially corrupt.
    if(pRepoData)
    {
        pr_err("Error: Failed to synchronize cache for repo '%s'\n",
            pRepoData->pszName);

        if(pTdnf)
        {
            TDNFRepoRemoveCache(pTdnf, pRepoData);
            TDNFRemoveSolvCache(pTdnf, pRepoData);
            TDNFRemoveLastRefreshMarker(pTdnf, pRepoData);
        }
    }
    goto cleanup;
}

uint32_t
TDNFInitRepoFromMetadata(
    Repo *pRepo,
    const char* pszRepoName,
    PTDNF_REPO_METADATA pRepoMD
    )
{
    uint32_t dwError = 0;

    if(!pRepo || !pRepoMD || IsNullOrEmptyString(pszRepoName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvReadYumRepo(pRepo,
                  pszRepoName,
                  pRepoMD->pszRepoMD,
                  pRepoMD->pszPrimary,
                  pRepoMD->pszFileLists,
                  pRepoMD->pszUpdateInfo,
                  pRepoMD->pszOther);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFInitCmdLineRepo(
    PTDNF pTdnf,
    PSolvSack pSack
    )
{
    uint32_t dwError = 0;
    Repo* pRepo = NULL;
    Pool* pPool = NULL;
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo = NULL;

    if(!pTdnf || !pTdnf->pConf || !pSack || !pSack->pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPool = pSack->pPool;

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(SOLV_REPO_INFO_INTERNAL),
                  (void**)&pSolvRepoInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepo = repo_create(pPool, CMDLINE_REPO_NAME);
    if (!pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSolvRepoInfo->pRepo = pRepo;
    pRepo->appdata = pSolvRepoInfo;

    pTdnf->pSolvCmdLineRepo = pRepo;

    pool_createwhatprovides(pPool);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pSolvRepoInfo);
    return dwError;
error:
    /*
     * coverty scan throws below warning
     * Execution cannot reach this statement: "repo_free(pRepo, 1);"
     * Ignoring this because it's good to have this condition check
     */
    if(pRepo)
    {
        /* coverity[dead_error_line] */
        repo_free(pRepo, 1);
    }
    goto cleanup;
}

uint32_t
TDNFGetGPGCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGCheck
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    int nGPGCheck = 0;

    if(!pTdnf || IsNullOrEmptyString(pszRepo) || !pnGPGCheck)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(!pTdnf->pArgs->nNoGPGCheck)
    {
        dwError = TDNFGetRepoById(pTdnf, pszRepo, &pRepo);
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
        if(pRepo)
        {
            nGPGCheck = pRepo->nGPGCheck;
        }
    }

    *pnGPGCheck = nGPGCheck;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGetSkipSignatureOption(
    PTDNF pTdnf,
    uint32_t *pdwSkipSignature
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwSkipSignature = 0;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(!strcasecmp(pSetOpt->pszOptName, "skipsignature"))
        {
            dwSkipSignature = 1;
            break;
        }
        pSetOpt = pSetOpt->pNext;
    }
    *pdwSkipSignature = dwSkipSignature;
cleanup:
    return dwError;

error:
    if(pdwSkipSignature)
    {
       *pdwSkipSignature = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetSkipDigestOption(
    PTDNF pTdnf,
    uint32_t *pdwSkipDigest
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwSkipDigest = 0;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(!strcasecmp(pSetOpt->pszOptName, "skipdigest"))
        {
            dwSkipDigest = 1;
            break;
        }
        pSetOpt = pSetOpt->pNext;
    }
    *pdwSkipDigest = dwSkipDigest;
cleanup:
    return dwError;

error:
    if(pdwSkipDigest)
    {
       *pdwSkipDigest = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetGPGSignatureCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGSigCheck,
    char*** pppszUrlGPGKeys
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    int nGPGSigCheck = 0;
    uint32_t dwSkipSignature = 0;
    char** ppszUrlGPGKeys = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepo) || !pnGPGSigCheck)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetSkipSignatureOption(pTdnf, &dwSkipSignature);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!(pTdnf->pArgs->nNoGPGCheck || dwSkipSignature))
    {
        dwError = TDNFGetRepoById(pTdnf, pszRepo, &pRepo);
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
        if(pRepo && pRepo->nGPGCheck)
        {
            nGPGSigCheck = 1;
            if (pppszUrlGPGKeys != NULL)
            {
                if (pRepo->ppszUrlGPGKeys == NULL ||
                    IsNullOrEmptyString(pRepo->ppszUrlGPGKeys[0]))
                {
                    dwError = ERROR_TDNF_NO_GPGKEY_CONF_ENTRY;
                    BAIL_ON_TDNF_ERROR(dwError);
                }
		dwError = TDNFAllocateStringArray(
                             pRepo->ppszUrlGPGKeys,
			     &ppszUrlGPGKeys);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    *pnGPGSigCheck = nGPGSigCheck;
    if (pppszUrlGPGKeys)
    {
        *pppszUrlGPGKeys = ppszUrlGPGKeys;
    }

cleanup:
    return dwError;

error:
    if(pppszUrlGPGKeys)
    {
        *pppszUrlGPGKeys = NULL;
    }
    TDNF_SAFE_FREE_STRINGARRAY(ppszUrlGPGKeys);
    goto cleanup;
}

uint32_t
TDNFGetRepoById(
    PTDNF pTdnf,
    const char* pszId,
    PTDNF_REPO_DATA* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszId) || !ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        if(!strcmp(pszId, pRepos->pszId))
        {
            pRepo = pRepos;
            break;
        }
        pRepos = pRepos->pNext;
    }

    if(!pRepo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppRepo = pRepo;
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFEventRepoMDDownloadEnd(
    PTDNF pTdnf,
    const char *pcszRepoId,
    const char *pcszRepoMDUrl,
    const char *pcszRepoMDFile
    )
{
    uint32_t dwError = 0;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf ||
        IsNullOrEmptyString(pcszRepoId) ||
        IsNullOrEmptyString(pcszRepoMDUrl) ||
        IsNullOrEmptyString(pcszRepoMDFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    stContext.nEvent = MAKE_PLUGIN_EVENT(
                           TDNF_PLUGIN_EVENT_TYPE_REPO_MD,
                           TDNF_PLUGIN_EVENT_STATE_DOWNLOAD,
                           TDNF_PLUGIN_EVENT_PHASE_END);
    dwError = TDNFAddEventDataString(&stContext,
                  TDNF_EVENT_ITEM_REPO_ID,
                  pcszRepoId);
    BAIL_ON_TDNF_ERROR(dwError);
    dwError = TDNFAddEventDataString(&stContext,
                  TDNF_EVENT_ITEM_REPO_MD_URL,
                  pcszRepoMDUrl);
    BAIL_ON_TDNF_ERROR(dwError);
    dwError = TDNFAddEventDataString(&stContext,
                  TDNF_EVENT_ITEM_REPO_MD_FILE,
                  pcszRepoMDFile);
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
TDNFStoreBaseURLFromMetalink(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszRepoMDURL
    )
{
    uint32_t dwError = 0;
    char *pszBaseUrlFile = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

    if (!pTdnf ||
        !pTdnf->pConf ||
        IsNullOrEmptyString(pszRepo) ||
        IsNullOrEmptyString(pszRepoMDURL))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_REPOS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFindRepoById(pTdnf, pszRepo, &pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               "tmp", TDNF_REPO_BASEURL_FILE_NAME,
                               &pszBaseUrlFile);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCreateAndWriteToFile(pszBaseUrlFile, pszRepoMDURL);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszBaseUrlFile);
    return dwError;
error:
    goto cleanup;
}

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

uint32_t
TDNFGetRepoMD(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    const char *pszRepoDataDir,
    PTDNF_REPO_METADATA *ppRepoMD
    )
{
    uint32_t dwError = 0;
    char *pszRepoMDFile = NULL;
    char *pszRepoMDUrl = NULL;
    char *pszMetaLinkFile = NULL;
    char *pszTmpRepoDataDir = NULL;
    char *pszTmpRepoMDFile = NULL;
    char *pszBaseUrlFile = NULL;
    char *pszTempBaseUrlFile = NULL;
    char* pszLastRefreshMarker = NULL;
    PTDNF_REPO_METADATA pRepoMDRel = NULL;
    PTDNF_REPO_METADATA pRepoMD = NULL;
    unsigned char pszMDCookie[SOLV_COOKIE_LEN] = {0};
    unsigned char pszTmpCookie[SOLV_COOKIE_LEN] = {0};
    int nNeedDownload = 0;
    int nNewRepoMDFile = 0;
    int nReplaceRepoMD = 0;
    int nKeepCache = 0;
    char *pszError = NULL;
    TDNF_ML_CTX *ml_ctx = NULL;

    if (!pTdnf ||
        !pRepoData ||
        IsNullOrEmptyString(pszRepoDataDir) ||
        !ppRepoMD)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pRepoData->pszMetaLink)) {
        dwError = TDNFJoinPath(&pszMetaLinkFile,
                               pszRepoDataDir,
                               TDNF_REPO_METALINK_FILE_NAME,
                               NULL);
        BAIL_ON_TDNF_ERROR(dwError);
        
        if (pTdnf->pArgs->nRefresh || access(pszMetaLinkFile, F_OK)) {
            dwError = TDNFUtilsMakeDirs(pszRepoDataDir);
            if (dwError == ERROR_TDNF_ALREADY_EXISTS)
            {
                dwError = 0;
            }
            BAIL_ON_TDNF_ERROR(dwError);
        
            dwError = TDNFDownloadFile(pTdnf, pRepoData->pszId, pRepoData->pszMetaLink,
                                       pszMetaLinkFile, pRepoData->pszId);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        dwError = TDNFAllocateMemory(1, sizeof(TDNF_ML_CTX),
                                     (void **)&ml_ctx);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFParseAndGetURLFromMetalink(pTdnf,
                    pszMetaLinkFile, ml_ctx);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFGetUrlsFromMLCtx(pTdnf, ml_ctx, &pRepoData->ppszBaseUrls);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!pRepoData->ppszBaseUrls || IsNullOrEmptyString(pRepoData->ppszBaseUrls[0]))
    {
        pr_err("Error: Cannot find a valid base URL for repo: %s\n", pRepoData->pszName);
        dwError = ERROR_TDNF_BASEURL_DOES_NOT_EXISTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nKeepCache = pTdnf->pConf->nKeepCache;

    dwError = TDNFJoinPath(&pszRepoMDFile,
                           pszRepoDataDir,
                           TDNF_REPO_METADATA_FILE_NAME,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_METADATA),
                  (void **)&pRepoMDRel);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetCachePath(pTdnf, pRepoData,
                               NULL, NULL,
                               &pRepoMDRel->pszRepoCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszRepoMDFile, &pRepoMDRel->pszRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pRepoData->pszId, &pRepoMDRel->pszRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszBaseUrlFile,
                           pszRepoDataDir,
                           TDNF_REPO_BASEURL_FILE_NAME,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    /* if repomd.xml file is not present, set flag to download */
    if (access(pszRepoMDFile, F_OK))
    {
        if (errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        nNeedDownload = 1;
    }

    /* if refresh flag is set, get shasum of existing repomd file */
    if (pTdnf->pArgs->nRefresh)
    {
        if (!access(pszRepoMDFile, F_OK))
        {
            dwError = SolvCalculateCookieForFile(pszRepoMDFile, pszMDCookie);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        nNeedDownload = 1;
    }

    /* download repomd.xml to tmp */
    if (nNeedDownload && !pTdnf->pArgs->nCacheOnly)
    {
        pr_info("Refreshing metadata for: '%s'\n", pRepoData->pszName);
        /* always download to tmp */
        dwError = TDNFGetCachePath(pTdnf, pRepoData,
                                   "tmp", NULL,
                                   &pszTmpRepoDataDir);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszTmpRepoDataDir);
        if (dwError == ERROR_TDNF_ALREADY_EXISTS)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFJoinPath(
                      &pszTmpRepoMDFile,
                      pszTmpRepoDataDir,
                      TDNF_REPO_METADATA_FILE_NAME,
                      NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadFileFromRepo(
                          pTdnf,
                          pRepoData,
                          TDNF_REPO_METADATA_FILE_PATH,
                          pszTmpRepoMDFile,
                          pRepoData->pszId);
        BAIL_ON_TDNF_ERROR(dwError);

        if (ml_ctx) {
            //check if the repomd file downloaded using metalink have the same checksum
            //as mentioned in the metalink file.
            dwError = TDNFCheckRepoMDFileHashFromMetalink(pszTmpRepoMDFile, ml_ctx);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        nReplaceRepoMD = 1;
        if (pszMDCookie[0])
        {
            dwError = SolvCalculateCookieForFile(pszTmpRepoMDFile, pszTmpCookie);
            BAIL_ON_TDNF_ERROR(dwError);
            if (!memcmp (pszMDCookie, pszTmpCookie, sizeof(pszTmpCookie)))
            {
                nReplaceRepoMD = 0;
            }
        }
        nNewRepoMDFile = 1;

        if (nNewRepoMDFile)
        {
            // FIXME
            dwError = TDNFJoinPath(&pszRepoMDUrl,
                                   pRepoData->ppszBaseUrls[0],
                                   TDNF_REPO_METADATA_FILE_PATH,
                                   NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            /* plugin event indicating a repomd download happened */
            dwError = TDNFEventRepoMDDownloadEnd(
                          pTdnf,
                          pRepoData->pszId,
                          pszRepoMDUrl,
                          pszTmpRepoMDFile);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    if (nReplaceRepoMD)
    {
        /* Remove the old repodata, solvcache and lastRefreshMarker before
           replacing the new repomd file and metalink files. */
        TDNFRepoRemoveCache(pTdnf, pRepoData);
        TDNFRemoveSolvCache(pTdnf, pRepoData);
        TDNFRemoveLastRefreshMarker(pTdnf, pRepoData);
        if (!nKeepCache)
        {
            TDNFRemoveRpmCache(pTdnf, pRepoData);
        }
        dwError = TDNFUtilsMakeDirs(pszRepoDataDir);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = TDNFReplaceFile(pszTmpRepoMDFile, pszRepoMDFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nNewRepoMDFile)
    {
        dwError = TDNFGetCachePath(pTdnf, pRepoData,
                                   TDNF_REPO_METADATA_MARKER, NULL,
                                   &pszLastRefreshMarker);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = TDNFTouchFile(pszLastRefreshMarker);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFParseRepoMD(pRepoMDRel);
    if (dwError == ERROR_TDNF_FILE_NOT_FOUND && pTdnf->pArgs->nCacheOnly)
    {
        dwError = ERROR_TDNF_CACHE_DISABLED;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFEnsureRepoMDParts(
                  pTdnf,
                  pRepoData,
                  pRepoMDRel,
                  &pRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);
    *ppRepoMD = pRepoMD;

cleanup:
    if (!IsNullOrEmptyString(pszTmpRepoDataDir))
    {
        if((TDNFRemoveTmpRepodata(pszTmpRepoDataDir)) &&
           (dwError == ERROR_TDNF_CHECKSUM_VALIDATION_FAILED))
	{
	    pr_crit("Downloaded repomd shasum mismatch, failed to remove %s file. Please remove it manually\n.",
                pszTmpRepoDataDir);
	}
    }
    TDNFFreeRepoMetadata(pRepoMDRel);
    TDNF_SAFE_FREE_MEMORY(pszTmpRepoMDFile);
    TDNF_SAFE_FREE_MEMORY(pszMetaLinkFile);
    TDNF_SAFE_FREE_MEMORY(pszTmpRepoDataDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDFile);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDUrl);
    TDNF_SAFE_FREE_MEMORY(pszBaseUrlFile);
    TDNF_SAFE_FREE_MEMORY(pszTempBaseUrlFile);
    TDNF_SAFE_FREE_MEMORY(pszError);
    TDNF_SAFE_FREE_MEMORY(pszLastRefreshMarker);
    if (ml_ctx)
    {
        TDNFMetalinkFree(ml_ctx);
        ml_ctx = NULL;
    }
    return dwError;

error:
    TDNFGetErrorString(dwError, &pszError);
    if (!IsNullOrEmptyString(pszError))
    {
        pr_err("Error(%u) : %s\n", dwError, pszError);
    }
    TDNFFreeRepoMetadata(pRepoMD);
    goto cleanup;
}

uint32_t
TDNFDownloadRepoMDPart(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszLocation,
    const char *pszDestPath
    )
{
    uint32_t dwError = 0;

    if(!pTdnf || !pRepo ||
       IsNullOrEmptyString(pszLocation) ||
       IsNullOrEmptyString(pszDestPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(access(pszDestPath, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }

        dwError = TDNFDownloadFileFromRepo(
                      pTdnf,
                      pRepo,
                      pszLocation,
                      pszDestPath,
                      pRepo->pszId);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFEnsureRepoMDParts(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    PTDNF_REPO_METADATA pRepoMDRel,
    PTDNF_REPO_METADATA *ppRepoMD
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_METADATA pRepoMD = NULL;

    if(!pTdnf || !pRepoMDRel || !ppRepoMD)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_METADATA),
                  (void **)&pRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);

    pRepoMD->pszRepoMD = pRepoMDRel->pszRepoMD;
    pRepoMDRel->pszRepoMD = NULL;

    dwError = TDNFAppendPath(
                  pRepoMDRel->pszRepoCacheDir,
                  pRepoMDRel->pszPrimary,
                  &pRepoMD->pszPrimary);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadRepoMDPart(
                  pTdnf,
                  pRepo,
                  pRepoMDRel->pszPrimary,
                  pRepoMD->pszPrimary);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!pRepo->nSkipMDFileLists && !IsNullOrEmptyString(pRepoMDRel->pszFileLists))
    {
        dwError = TDNFAppendPath(
                      pRepoMDRel->pszRepoCacheDir,
                      pRepoMDRel->pszFileLists,
                      &pRepoMD->pszFileLists);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadRepoMDPart(
                      pTdnf,
                      pRepo,
                      pRepoMDRel->pszFileLists,
                      pRepoMD->pszFileLists);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pRepo->nSkipMDUpdateInfo && !IsNullOrEmptyString(pRepoMDRel->pszUpdateInfo))
    {
        dwError = TDNFAppendPath(
                      pRepoMDRel->pszRepoCacheDir,
                      pRepoMDRel->pszUpdateInfo,
                      &pRepoMD->pszUpdateInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadRepoMDPart(
                      pTdnf,
                      pRepo,
                      pRepoMDRel->pszUpdateInfo,
                      pRepoMD->pszUpdateInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pRepo->nSkipMDOther && !IsNullOrEmptyString(pRepoMDRel->pszOther))
    {
        dwError = TDNFAppendPath(
                      pRepoMDRel->pszRepoCacheDir,
                      pRepoMDRel->pszOther,
                      &pRepoMD->pszOther);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadRepoMDPart(
                      pTdnf,
                      pRepo,
                      pRepoMDRel->pszOther,
                      pRepoMD->pszOther);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppRepoMD = pRepoMD;

cleanup:
    return dwError;

error:
    TDNFFreeRepoMetadata(pRepoMD);
    goto cleanup;
}

uint32_t
TDNFFindRepoMDPart(
    Repo *pSolvRepo,
    const char *pszType,
    char **ppszPart
    )
{
    uint32_t dwError = 0;
    Pool *pPool = NULL;
    Dataiterator di = {0};
    char *pszPart = NULL;
    const char *pszPartTemp = NULL;

    if(!pSolvRepo ||
       !pSolvRepo->pool ||
       IsNullOrEmptyString(pszType) ||
       !ppszPart)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPool = pSolvRepo->pool;

    dwError = dataiterator_init(
                  &di,
                  pPool,
                  pSolvRepo,
                  SOLVID_META,
                  REPOSITORY_REPOMD_TYPE,
                  pszType,
                  SEARCH_STRING);
    BAIL_ON_TDNF_ERROR(dwError);

    dataiterator_prepend_keyname(&di, REPOSITORY_REPOMD);
    if (dataiterator_step(&di))
    {
        dataiterator_setpos_parent(&di);
        pszPartTemp = pool_lookup_str(
                          pPool,
                          SOLVID_POS,
                          REPOSITORY_REPOMD_LOCATION);
    }

    if(!pszPartTemp)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPartTemp, &pszPart);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszPart = pszPart;

cleanup:
    dataiterator_free(&di);
    return dwError;

error:
    if(ppszPart)
    {
        *ppszPart = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszPart);
    goto cleanup;
}

uint32_t
TDNFParseRepoMD(
    PTDNF_REPO_METADATA pRepoMD
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    Pool *pPool = NULL;
    FILE *fp = NULL;

    if(!pRepoMD)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPool = pool_create();
    pRepo = repo_create(pPool, "md_parse_temp");

    fp = fopen(pRepoMD->pszRepoMD, "r");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = repo_add_repomdxml(pRepo, fp, 0);
    if(dwError)
    {
        pr_crit("Error(%u) parsing repomd: %s\n",
                dwError,
                pool_errstr(pPool));
    }

    dwError = TDNFFindRepoMDPart(
                  pRepo,
                  TDNF_REPOMD_TYPE_PRIMARY,
                  &pRepoMD->pszPrimary);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFFindRepoMDPart(
                  pRepo,
                  TDNF_REPOMD_TYPE_FILELISTS,
                  &pRepoMD->pszFileLists);
    /* file lists can be missing (issue #273) */
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFFindRepoMDPart(
                  pRepo,
                  TDNF_REPOMD_TYPE_UPDATEINFO,
                  &pRepoMD->pszUpdateInfo);
    /* updateinfo is not mandatory */
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFFindRepoMDPart(
                  pRepo,
                  TDNF_REPOMD_TYPE_OTHER,
                  &pRepoMD->pszOther);
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    if(pRepo)
    {
        repo_free(pRepo, 0);
    }
    if(pPool)
    {
        pool_free(pPool);
    }
    return dwError;

error:
    goto cleanup;
}

void
TDNFFreeRepoMetadata(
    PTDNF_REPO_METADATA pRepoMD
    )
{
    if(!pRepoMD)
    {
        return;
    }
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszRepoCacheDir);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszRepo);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszRepoMD);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszPrimary);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszFileLists);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszUpdateInfo);
    TDNF_SAFE_FREE_MEMORY(pRepoMD->pszOther);
    TDNF_SAFE_FREE_MEMORY(pRepoMD);
}

uint32_t
TDNFReplaceFile(
    const char *pszSrcFile,
    const char *pszDstFile
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszSrcFile) || IsNullOrEmptyString(pszDstFile) || access(pszSrcFile, F_OK))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    /* coverity[toctou] */
    if (rename(pszSrcFile, pszDstFile) == -1)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadMetadata(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszRepoDir,
    int nPrintOnly
    )
{
    uint32_t dwError = 0;
    char *pszRepoMDPath = NULL;
    char *pszRepoMDUrl = NULL;
    char *pszRepoDataDir = NULL;
    Repo *pSolvRepo = NULL;
    Pool *pPool = NULL;
    FILE *fp = NULL;

    if (!nPrintOnly)
    {
        dwError = TDNFUtilsMakeDir(pszRepoDir);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFJoinPath(&pszRepoDataDir,
                    pszRepoDir,
                    "repodata",
                    NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFUtilsMakeDir(pszRepoDataDir);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFJoinPath(&pszRepoMDPath,
                    pszRepoDataDir, TDNF_REPO_METADATA_FILE_NAME,
                    NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadFileFromRepo(pTdnf, pRepo, TDNF_REPO_METADATA_FILE_PATH, pszRepoMDPath, pRepo->pszId);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        /* use first base url - we cannot tell which one is good */
        dwError = TDNFJoinPath(&pszRepoMDUrl,
                               pRepo->ppszBaseUrls[0],
                               TDNF_REPO_METADATA_FILE_PATH,
                               NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        /* if printing only we use the already downloaded repomd.xml */
        dwError = TDNFGetCachePath(pTdnf, pRepo,
                                   TDNF_REPO_METADATA_FILE_PATH, NULL,
                                   &pszRepoMDPath);
        BAIL_ON_TDNF_ERROR(dwError);

        pr_info("%s\n", pszRepoMDUrl);
    }

    pPool = pool_create();
    pSolvRepo = repo_create(pPool, "md_parse_temp");

    fp = fopen(pszRepoMDPath, "r");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = repo_add_repomdxml(pSolvRepo, fp, 0);
    if(dwError)
    {
        pr_crit("Error(%u) parsing repomd: %s\n",
                dwError,
                pool_errstr(pPool));
    }

    dwError = TDNFDownloadRepoMDParts(pTdnf, pSolvRepo, pRepo, pszRepoDir, nPrintOnly);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    if (pSolvRepo)
    {
        repo_free(pSolvRepo, 0);
    }
    if (pPool)
    {
        pool_free(pPool);
    }
    TDNF_SAFE_FREE_MEMORY(pszRepoMDPath);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDUrl);
    TDNF_SAFE_FREE_MEMORY(pszRepoDataDir);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadRepoMDParts(
    PTDNF pTdnf,
    Repo *pSolvRepo,
    PTDNF_REPO_DATA pRepo,
    const char *pszDir,
    int nPrintOnly
    )
{
    uint32_t dwError = 0;
    Pool *pPool = NULL;
    Dataiterator di = {0};
    const char *pszPartFile = NULL;
    char *pszPartUrl = NULL;
    char *pszPartPath = NULL;

    if(!pSolvRepo ||
       !pSolvRepo->pool ||
       !pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPool = pSolvRepo->pool;

    dwError = dataiterator_init(
                  &di,
                  pPool,
                  pSolvRepo,
                  SOLVID_META,
                  REPOSITORY_REPOMD_TYPE,
                  0,
                  SEARCH_STRING);
    BAIL_ON_TDNF_ERROR(dwError);

    dataiterator_prepend_keyname(&di, REPOSITORY_REPOMD);

    while (dataiterator_step(&di))
    {
        dataiterator_setpos_parent(&di);
        pszPartFile = pool_lookup_str(
                          pPool,
                          SOLVID_POS,
                          REPOSITORY_REPOMD_LOCATION);

        dwError = TDNFJoinPath(&pszPartPath,
                               pszDir,
                               pszPartFile,
                               NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        if (!nPrintOnly)
        {
            dwError = TDNFDownloadFileFromRepo(pTdnf, pRepo, pszPartFile, pszPartPath, pRepo->pszId);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else
        {
            dwError = TDNFJoinPath(&pszPartUrl,
                                   pRepo->ppszBaseUrls[0],
                                   pszPartFile,
                                   NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            pr_info("%s\n", pszPartUrl);
            TDNF_SAFE_FREE_MEMORY(pszPartUrl);
        }
        TDNF_SAFE_FREE_MEMORY(pszPartPath);
    }

cleanup:
    dataiterator_free(&di);
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszPartUrl);
    TDNF_SAFE_FREE_MEMORY(pszPartPath);

    goto cleanup;
}

