/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
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
    PTDNF_REPO_DATA_INTERNAL pRepoData,
    PSolvSack pSack
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;
    char* pszRepoDataDir = NULL;
    char* pszLastRefreshMarker = NULL;
    PTDNF_REPO_METADATA pRepoMD = NULL;
    PTDNF_CONF pConf = NULL;
    Repo* pRepo = NULL;
    Pool* pPool = NULL;;
    int nUseMetaDataCache = 0;
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo = NULL;

    if(!pTdnf || !pTdnf->pConf || !pRepoData || !pSack || !pSack->pPool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pConf = pTdnf->pConf;
    pPool = pSack->pPool;

    dwError = TDNFAllocateStringPrintf(
                  &pszRepoCacheDir,
                  "%s/%s",
                  pConf->pszCacheDir,
                  pRepoData->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(
                  &pszRepoDataDir,
                  "%s/%s",
                  pszRepoCacheDir,
                  TDNF_REPODATA_DIR_NAME);
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
    pRepo->appdata = pSolvRepoInfo;

    dwError = SolvCalculateCookieForRepoMD(pRepoMD->pszRepoMD, pSolvRepoInfo->cookie);
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
    dwError = TDNFAllocateStringPrintf(
                  &pszLastRefreshMarker,
                  "%s/%s",
                  pszRepoCacheDir,
                  TDNF_REPO_METADATA_MARKER);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFTouchFile(pszLastRefreshMarker);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNFFreeRepoMetadata(pRepoMD);
    TDNF_SAFE_FREE_MEMORY(pszLastRefreshMarker);
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
        fprintf(
            stderr,
            "Error: Failed to synchronize cache for repo '%s' from '%s'\n",
            pRepoData->pszName,
            pRepoData->pszBaseUrl);

        if(pTdnf)
        {
            TDNFRepoRemoveCache(pTdnf, pRepoData->pszId);
            TDNFRemoveSolvCache(pTdnf, pRepoData->pszId);
            TDNFRemoveLastRefreshMarker(pTdnf, pRepoData->pszId);
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
                  pRepoMD->pszUpdateInfo);
cleanup:
    return dwError;

error:
    goto cleanup;
    return dwError;
}

uint32_t
TDNFGetGPGCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGCheck,
    char** ppszUrlGPGKey
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;
    int nGPGCheck = 0;
    char* pszUrlGPGKey = NULL;

    if(!pTdnf || !ppszUrlGPGKey || IsNullOrEmptyString(pszRepo) || !pnGPGCheck)
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
            if(nGPGCheck)
            {
                dwError = TDNFAllocateString(
                             pRepo->pszUrlGPGKey,
                             &pszUrlGPGKey);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    *pnGPGCheck = nGPGCheck;
    *ppszUrlGPGKey = pszUrlGPGKey;

cleanup:
    return dwError;

error:
    if(ppszUrlGPGKey)
    {
        *ppszUrlGPGKey = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszUrlGPGKey);
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
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
          !strcasecmp(pSetOpt->pszOptName, "skipsignature"))
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
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
          !strcasecmp(pSetOpt->pszOptName, "skipdigest"))
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
    char** ppszUrlGPGKey
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;
    int nGPGSigCheck = 0;
    uint32_t dwSkipSignature = 0;
    char* pszUrlGPGKey = NULL;

    if(!pTdnf || !ppszUrlGPGKey || IsNullOrEmptyString(pszRepo) || !pnGPGSigCheck)
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
        if(pRepo)
        {
            nGPGSigCheck = pRepo->nGPGCheck;
            if(nGPGSigCheck)
            {
                dwError = TDNFAllocateString(
                             pRepo->pszUrlGPGKey,
                             &pszUrlGPGKey);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

    *pnGPGSigCheck = nGPGSigCheck;
    *ppszUrlGPGKey = pszUrlGPGKey;

cleanup:
    return dwError;

error:
    if(ppszUrlGPGKey)
    {
        *ppszUrlGPGKey = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszUrlGPGKey);
    goto cleanup;
}

uint32_t
TDNFGetRepoById(
    PTDNF pTdnf,
    const char* pszId,
    PTDNF_REPO_DATA_INTERNAL* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepo = NULL;
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;

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
TDNFGetRepoMD(
    PTDNF pTdnf,
    PTDNF_REPO_DATA_INTERNAL pRepoData,
    const char *pszRepoDataDir,
    PTDNF_REPO_METADATA *ppRepoMD
    )
{
    uint32_t dwError = 0;
    char *pszRepoMDFile = NULL;
    char *pszRepoMDUrl = NULL;
    char *pszTmpRepoDataDir = NULL;
    char *pszTmpRepoMDFile = NULL;
    PTDNF_REPO_METADATA pRepoMDRel = NULL;
    PTDNF_REPO_METADATA pRepoMD = NULL;
    unsigned char pszCookie[SOLV_COOKIE_LEN] = {0};
    unsigned char pszTmpCookie[SOLV_COOKIE_LEN] = {0};
    int nNeedDownload = 0;
    int nNewRepoMDFile = 0;
    int nReplaceRepoMD = 0;

    if(!pTdnf ||
       !pRepoData ||
       IsNullOrEmptyString(pszRepoDataDir) ||
       !ppRepoMD)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateStringPrintf(&pszRepoMDFile,
                                      "%s/%s",
                                      pszRepoDataDir,
                                      TDNF_REPO_METADATA_FILE_NAME);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pszRepoMDUrl,
                                      "%s/%s",
                                      pRepoData->pszBaseUrl,
                                      TDNF_REPO_METADATA_FILE_PATH);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_REPO_METADATA),
                  (void **)&pRepoMDRel);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(
                  &pRepoMDRel->pszRepoCacheDir,
                  "%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pRepoData->pszId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszRepoMDFile, &pRepoMDRel->pszRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pRepoData->pszId, &pRepoMDRel->pszRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    /* if repomd.xml file is not present, set flag to download */
    if (access(pszRepoMDFile, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        nNeedDownload = 1;
    }
    /* if refresh flag is set, get shasum of existing repomd file */
    else if (pTdnf->pArgs->nRefresh)
    {
        dwError = SolvCalculateCookieForRepoMD(pszRepoMDFile, pszCookie);
        BAIL_ON_TDNF_ERROR(dwError);
        nNeedDownload = 1;
    }

    /* download repomd.xml to tmp */
    if(nNeedDownload)
    {
        if(!pTdnf->pArgs->nQuiet)
        {
           printf("Refreshing metadata for: '%s'\n",
                    pRepoData->pszName);
        }
        /* always download to tmp */
        dwError = TDNFAllocateStringPrintf(
                      &pszTmpRepoDataDir,
                      "%s/tmp",
                      pszRepoDataDir);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszTmpRepoDataDir);
        if(dwError == ERROR_TDNF_ALREADY_EXISTS)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateStringPrintf(
                      &pszTmpRepoMDFile,
                      "%s/%s",
                      pszTmpRepoDataDir,
                      TDNF_REPO_METADATA_FILE_NAME);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadFile(
                      pTdnf,
                      pRepoData->pszId,
                      pszRepoMDUrl,
                      pszTmpRepoMDFile,
                      pRepoData->pszId);
        BAIL_ON_TDNF_ERROR(dwError);

        /* plugin event indicating a repomd download happened */
        dwError = TDNFEventRepoMDDownloadEnd(
                      pTdnf,
                      pRepoData->pszId,
                      pszRepoMDUrl,
                      pszTmpRepoMDFile);
        BAIL_ON_TDNF_ERROR(dwError);

        nNewRepoMDFile = 1;
    }

    if (nNewRepoMDFile)
    {
        nReplaceRepoMD = 1;
        /* compare shasum if applicable */
        if (pszCookie[0])
        {
            dwError = SolvCalculateCookieForRepoMD(pszTmpRepoMDFile, pszTmpCookie);
            BAIL_ON_TDNF_ERROR(dwError);

            if (!memcmp (pszCookie, pszTmpCookie, sizeof(pszTmpCookie)))
            {
                nReplaceRepoMD = 0;
            }
        }

        if (nReplaceRepoMD)
        {
            dwError = TDNFReplaceRepoMDFile(pszTmpRepoMDFile, pszRepoMDFile);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFParseRepoMD(pRepoMDRel);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFEnsureRepoMDParts(
                  pTdnf,
                  pRepoData->pszBaseUrl,
                  pRepoMDRel,
                  &pRepoMD);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppRepoMD = pRepoMD;

cleanup:
    TDNFFreeRepoMetadata(pRepoMDRel);
    TDNFRemoveTmpRepodata(pszTmpRepoDataDir, pszTmpRepoMDFile);
    TDNF_SAFE_FREE_MEMORY(pszTmpRepoMDFile);
    TDNF_SAFE_FREE_MEMORY(pszTmpRepoDataDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDFile);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDUrl);
    return dwError;

error:
    fprintf(stderr, "Error: %s %d\n", __FUNCTION__, dwError);
    TDNFFreeRepoMetadata(pRepoMD);
    goto cleanup;
}

uint32_t
TDNFDownloadRepoMDPart(
    PTDNF pTdnf,
    const char *pszBaseUrl,
    const char *pszRepo,
    const char *pszFileName,
    const char *pszDestPath
    )
{
    uint32_t dwError = 0;
    char *pszTempUrl = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszBaseUrl) ||
       IsNullOrEmptyString(pszRepo) ||
       IsNullOrEmptyString(pszFileName) ||
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

        dwError = TDNFAppendPath(
                      pszBaseUrl,
                      pszFileName,
                      &pszTempUrl);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadFile(
                      pTdnf,
                      pszRepo,
                      pszTempUrl,
                      pszDestPath,
                      pszRepo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszTempUrl);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFEnsureRepoMDParts(
    PTDNF pTdnf,
    const char *pszBaseUrl,
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
                  pszBaseUrl,
                  pRepoMDRel->pszRepo,
                  pRepoMDRel->pszPrimary,
                  pRepoMD->pszPrimary);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAppendPath(
                  pRepoMDRel->pszRepoCacheDir,
                  pRepoMDRel->pszFileLists,
                  &pRepoMD->pszFileLists);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadRepoMDPart(
                  pTdnf,
                  pszBaseUrl,
                  pRepoMDRel->pszRepo,
                  pRepoMDRel->pszFileLists,
                  pRepoMD->pszFileLists);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pRepoMDRel->pszUpdateInfo))
    {
        dwError = TDNFAppendPath(
                      pRepoMDRel->pszRepoCacheDir,
                      pRepoMDRel->pszUpdateInfo,
                      &pRepoMD->pszUpdateInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFDownloadRepoMDPart(
                      pTdnf,
                      pszBaseUrl,
                      pRepoMDRel->pszRepo,
                      pRepoMDRel->pszUpdateInfo,
                      pRepoMD->pszUpdateInfo);
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
        printf("Error(%u) parsing repomd: %s\n",
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
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFFindRepoMDPart(
                  pRepo,
                  TDNF_REPOMD_TYPE_UPDATEINFO,
                  &pRepoMD->pszUpdateInfo);
    //updateinfo is not mandatory
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
    TDNF_SAFE_FREE_MEMORY(pRepoMD);
}

uint32_t
TDNFReplaceRepoMDFile(
    const char *pszSrcFile,
    const char *pszDstFile
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszSrcFile) || IsNullOrEmptyString(pszDstFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = rename (pszSrcFile, pszDstFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}
