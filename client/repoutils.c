/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <glob.h>


uint32_t
TDNFRepoGetUserPass(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    char** ppszUserPass
    )
{
    uint32_t dwError = 0;
    char* pszUserPass = NULL;

    if(!pTdnf || !pRepo || !ppszUserPass)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pRepo->pszUser) &&
       !IsNullOrEmptyString(pRepo->pszPass))
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszUserPass,
                      "%s:%s",
                      pRepo->pszUser,
                      pRepo->pszPass);
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
    PTDNF_REPO_DATA pRepo,
    char** ppszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;

    if(!pTdnf || !pRepo || !ppszRpmCacheDir)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pTdnf->pConf || IsNullOrEmptyString(pTdnf->pConf->pszCacheDir))
    {
        dwError = ERROR_TDNF_INVALID_CONF;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               TDNF_RPM_CACHE_DIR_NAME, NULL,
                               &pszRpmCacheDir);
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

/* remove the repo top level cache dir */
uint32_t
TDNFRepoRemoveCacheDir(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;

    if(!pTdnf || !pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               NULL, NULL,
                               &pszRepoCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if (rmdir(pszRepoCacheDir) != 0 && errno != ENOENT)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoRemoveCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;

    if(!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               TDNF_REPODATA_DIR_NAME, NULL,
                               &pszRepoCacheDir);
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
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;

    if (!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetRpmCacheDir(pTdnf, pRepo, &pszRpmCacheDir);
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

static
uint32_t
_TDNFRemoveRepoCacheFile(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszFilename
    )
{
    uint32_t dwError = 0;
    char *pszPath = NULL;

    if(!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               pszFilename, NULL,
                               &pszPath);
    BAIL_ON_TDNF_ERROR(dwError);
    if (pszPath)
    {
        if(unlink(pszPath) && errno != ENOENT)
        {
           dwError = errno;
           BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPath);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRemoveLastRefreshMarker(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    return _TDNFRemoveRepoCacheFile(pTdnf, pRepo, TDNF_REPO_METADATA_MARKER);
}

uint32_t
TDNFRemoveMirrorList(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    return _TDNFRemoveRepoCacheFile(pTdnf, pRepo, TDNF_REPO_METADATA_MIRRORLIST);
}

uint32_t
TDNFRemoveSnapshot(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    int i, ret = 0;
    char *pszPathPattern = NULL;
    glob_t globbuf =  {0};

    if(!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* glob for all files matching TDNF_REPO_METADATA_MIRRORLIST"-*" ("snapshot*")
       and remove them */
    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               TDNF_REPO_METADATA_SNAPSHOT"-*", NULL,
                               &pszPathPattern);
    BAIL_ON_TDNF_ERROR(dwError);

    ret = glob(pszPathPattern, 0, NULL, &globbuf);
    if (ret == 0) {
        for (i = 0; globbuf.gl_pathv[i]; i++) {
            if(unlink(globbuf.gl_pathv[i]) && errno != ENOENT)
            {
               dwError = errno;
               BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
        }
    }

cleanup:
    globfree(&globbuf);
    TDNF_SAFE_FREE_MEMORY(pszPathPattern);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRemoveSolvCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    char* pszSolvCacheDir = NULL;

    if(!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               TDNF_SOLVCACHE_DIR_NAME, NULL,
                               &pszSolvCacheDir);
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
    PTDNF_REPO_DATA pRepo
    )
{
    uint32_t dwError = 0;
    char* pszKeysDir = NULL;

    if(!pTdnf || !pRepo || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               "keys", NULL,
                               &pszKeysDir);
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
            ((pRepo->nSSLVerify) ? 1L : 0L)) != CURLE_OK)
    {
        dwError = ERROR_TDNF_SET_SSL_SETTINGS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(curl_easy_setopt(
            pCurl,
            CURLOPT_SSL_VERIFYHOST,
            ((pRepo->nSSLVerify) ? 2L : 0L)) != CURLE_OK)
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
TDNFGetCachePath(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszSubDir,
    const char *pszFileName,
    char **ppszPath
)
{
    uint32_t dwError = 0;

    if(!pTdnf || !pRepo || !ppszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(
                  ppszPath,
                  pTdnf->pConf->pszCacheDir,
                  pRepo->pszCacheName ? pRepo->pszCacheName : pRepo->pszId,
                  pszSubDir,
                  pszFileName,
                  NULL);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

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
