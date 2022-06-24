/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : repoutils.c
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
TDNFRepoMakeCacheDirs(
    const char* pszRepo
    )
{
    uint32_t dwError = 0;
    if(IsNullOrEmptyString(pszRepo))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoSetBaseUrl(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pszRepo,
    const char *pszBaseUrlFile
    )
{
    uint32_t dwError = 0;
    char *pszBaseUrl = NULL;

    if (!pTdnf || !pszRepo || IsNullOrEmptyString(pszBaseUrlFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    TDNF_SAFE_FREE_MEMORY(pszRepo->pszBaseUrl);
    dwError = TDNFFileReadAllText(pszBaseUrlFile, &pszBaseUrl, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    pszRepo->pszBaseUrl = pszBaseUrl;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoGetBaseUrl(
    PTDNF pTdnf,
    const char* pszRepo,
    char** ppszBaseUrl
    )
{
    uint32_t dwError = 0;
    char* pszBaseUrl = NULL;
    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepo) || !ppszBaseUrl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_REPOS;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        if(!strcmp(pszRepo, pRepos->pszId))
        {
            break;
        }
        pRepos = pRepos->pNext;
    }

    if(!pRepos)
    {
        dwError = ERROR_TDNF_REPO_NOT_FOUND;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pRepos->pszBaseUrl) {
        dwError = TDNFAllocateString(pRepos->pszBaseUrl, &pszBaseUrl);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszBaseUrl = pszBaseUrl;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszBaseUrl);
    goto cleanup;
}

uint32_t
TDNFRepoGetUserPass(
    PTDNF pTdnf,
    const char* pszRepo,
    char** ppszUserPass
    )
{
    uint32_t dwError = 0;
    char* pszUserPass = NULL;
    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepo) || !ppszUserPass)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_REPOS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pRepos = pTdnf->pRepos; pRepos; pRepos = pRepos->pNext)
    {
        if (!strcmp(pszRepo, pRepos->pszId))
        {
            break;
        }
    }
    if (!pRepos)
    {
        dwError = ERROR_TDNF_REPO_NOT_FOUND;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pRepos->pszUser) &&
       !IsNullOrEmptyString(pRepos->pszPass))
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszUserPass,
                      "%s:%s",
                      pRepos->pszUser,
                      pRepos->pszPass);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszUserPass = pszUserPass;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
    goto cleanup;
}

uint32_t
TDNFRepoGetRpmCacheDir(
    PTDNF pTdnf,
    const char* pszRepoId,
    char** ppszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepoId) || !ppszRpmCacheDir)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pTdnf->pConf || IsNullOrEmptyString(pTdnf->pConf->pszCacheDir))
    {
        dwError = ERROR_TDNF_INVALID_CONF;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  &pszRpmCacheDir,
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_RPM_CACHE_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    if(access(pszRpmCacheDir, F_OK))
    {
        dwError = errno;
        if (dwError == ENOENT)
        {
            dwError = 0;
            TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
            pszRpmCacheDir = NULL;
        }
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    *ppszRpmCacheDir = pszRpmCacheDir;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    goto cleanup;
}

uint32_t
TDNFRepoRemoveCache(
    PTDNF pTdnf,
    const char* pszRepoId
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  &pszRepoCacheDir,
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_REPODATA_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRecursivelyRemoveDir(pszRepoCacheDir);
    if (dwError != ERROR_TDNF_SYSTEM_BASE + ENOENT)
    {
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRemoveRpmCache(
    PTDNF pTdnf,
    const char* pszRepoId
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;

    if (!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetRpmCacheDir(pTdnf, pszRepoId, &pszRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!IsNullOrEmptyString(pszRpmCacheDir))
    {
        dwError = TDNFRecursivelyRemoveDir(pszRpmCacheDir);
        if (dwError != ERROR_TDNF_SYSTEM_BASE + ENOENT)
        {
            BAIL_ON_TDNF_ERROR(dwError);
        }
        dwError = 0;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRemoveTmpRepodata(
    const char* pszTmpRepodataDir
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszTmpRepodataDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRecursivelyRemoveDir(pszTmpRepodataDir);
    if (dwError != ERROR_TDNF_SYSTEM_BASE + ENOENT)
    {
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRemoveLastRefreshMarker(
    PTDNF pTdnf,
    const char* pszRepoId
    )
{
    uint32_t dwError = 0;
    char* pszLastRefreshMarker = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  &pszLastRefreshMarker,
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_REPO_METADATA_MARKER,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);
    if (pszLastRefreshMarker)
    {
        if(unlink(pszLastRefreshMarker) && errno != ENOENT)
        {
           dwError = errno;
           BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszLastRefreshMarker);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRemoveSolvCache(
    PTDNF pTdnf,
    const char* pszRepoId
    )
{
    uint32_t dwError = 0;
    char* pszSolvCacheDir = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  &pszSolvCacheDir,
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_SOLVCACHE_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRecursivelyRemoveDir(pszSolvCacheDir);
    if (dwError != ERROR_TDNF_SYSTEM_BASE + ENOENT)
    {
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszSolvCacheDir);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRemoveKeysCache(
    PTDNF pTdnf,
    const char* pszRepoId
    )
{
    uint32_t dwError = 0;
    char* pszKeysDir = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  &pszKeysDir,
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  "keys",
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRecursivelyRemoveDir(pszKeysDir);
    if (dwError != ERROR_TDNF_SYSTEM_BASE + ENOENT)
    {
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszKeysDir);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoApplyProxySettings(
    PTDNF_CONF pConf,
    CURL *pCurl
    )
{
    uint32_t dwError = 0;

    if(!pConf || !pCurl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pConf->pszProxy))
    {
        if(curl_easy_setopt(
               pCurl,
               CURLOPT_PROXY,
               pConf->pszProxy) != CURLE_OK)
        {
            dwError = ERROR_TDNF_SET_PROXY;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        if(!IsNullOrEmptyString(pConf->pszProxyUserPass))
        {
            if(curl_easy_setopt(
                   pCurl,
                   CURLOPT_PROXYUSERPWD,
                   pConf->pszProxyUserPass) != CURLE_OK)
            {
                dwError = ERROR_TDNF_SET_PROXY_USERPASS;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRepoApplyDownloadSettings(
    PTDNF_REPO_DATA pRepo,
    CURL *pCurl
    )
{
    CURLcode curlError = CURLE_OK;
    uint32_t dwError = 0;

    if(!pRepo || !pCurl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if((curlError = curl_easy_setopt(
            pCurl,
            CURLOPT_TIMEOUT,
            pRepo->nTimeout)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_CURL_BASE + curlError;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if((curlError = curl_easy_setopt(
            pCurl,
            CURLOPT_LOW_SPEED_TIME,
            pRepo->nTimeout)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_CURL_BASE + curlError;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if((curlError = curl_easy_setopt(
            pCurl,
            CURLOPT_LOW_SPEED_LIMIT,
            pRepo->nMinrate)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_CURL_BASE + curlError;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if((curlError = curl_easy_setopt(
            pCurl,
            CURLOPT_MAX_RECV_SPEED_LARGE,
            pRepo->nThrottle)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_CURL_BASE + curlError;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRepoApplySSLSettings(
    PTDNF_REPO_DATA pRepo,
    CURL *pCurl
    )
{
    uint32_t dwError = 0;

    if(!pRepo || !pCurl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(curl_easy_setopt(
            pCurl,
            CURLOPT_SSL_VERIFYPEER,
            ((pRepo->nSSLVerify) ? 1 : 0)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_SET_SSL_SETTINGS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(curl_easy_setopt(
            pCurl,
            CURLOPT_SSL_VERIFYHOST,
            ((pRepo->nSSLVerify) ? 2 : 0)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_SET_SSL_SETTINGS;
        BAIL_ON_TDNF_ERROR(dwError);
    }


    if(!IsNullOrEmptyString(pRepo->pszSSLCaCert))
    {
        if(curl_easy_setopt(
                pCurl,
                CURLOPT_CAINFO,
                pRepo->pszSSLCaCert) != CURLE_OK)
        {
            dwError = ERROR_TDNF_SET_SSL_SETTINGS;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    if(!IsNullOrEmptyString(pRepo->pszSSLClientCert))
    {
        if(curl_easy_setopt(
                pCurl,
                CURLOPT_SSLCERT,
                pRepo->pszSSLClientCert) != CURLE_OK)
        {
            dwError = ERROR_TDNF_SET_SSL_SETTINGS;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    if(!IsNullOrEmptyString(pRepo->pszSSLClientKey))
    {
        if(curl_easy_setopt(
                pCurl,
                CURLOPT_SSLKEY,
                pRepo->pszSSLClientKey) != CURLE_OK)
        {
            dwError = ERROR_TDNF_SET_SSL_SETTINGS;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFFindRepoById(
    PTDNF pTdnf,
    const char* pszRepo,
    PTDNF_REPO_DATA* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepo))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_REPOS;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        if(!strcmp(pszRepo, pRepos->pszId))
        {
            break;
        }
        pRepos = pRepos->pNext;
    }

    if(!pRepos)
    {
        dwError = ERROR_TDNF_REPO_NOT_FOUND;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppRepo = pRepos;

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFCurlErrorIsFatal(
    CURLcode curlError
)
{
    uint32_t dwError = 0;

    switch(curlError) {
    case CURLE_UNSUPPORTED_PROTOCOL:
    case CURLE_FAILED_INIT:
    case CURLE_URL_MALFORMAT:
    case CURLE_FILE_COULDNT_READ_FILE:
    case CURLE_FUNCTION_NOT_FOUND:
    case CURLE_UNKNOWN_OPTION:
    case CURLE_SSL_ENGINE_NOTFOUND:
    case CURLE_RECURSIVE_API_CALL:

    case CURLE_ABORTED_BY_CALLBACK:
    case CURLE_BAD_FUNCTION_ARGUMENT:
    case CURLE_CONV_REQD:
    case CURLE_COULDNT_RESOLVE_PROXY:
    case CURLE_FILESIZE_EXCEEDED:
    case CURLE_INTERFACE_FAILED:
    case CURLE_NOT_BUILT_IN:
    case CURLE_OUT_OF_MEMORY:
    case CURLE_SSL_CACERT_BADFILE:
    case CURLE_SSL_CRL_BADFILE:
    case CURLE_WRITE_ERROR:
        dwError = curlError;
        break;
    default:
        break;
    }
    return dwError;
}
