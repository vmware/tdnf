/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
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
    PTDNF_REPO_DATA_INTERNAL pszRepo,
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
    dwError = TDNFFileReadAllText(pszBaseUrlFile, &pszBaseUrl);
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
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;

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
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;

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

    dwError = TDNFAllocateStringPrintf(
                  &pszRpmCacheDir,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_RPM_CACHE_DIR_NAME);
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
    char* pszFilePath = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateStringPrintf(
                  &pszRepoCacheDir,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_REPODATA_DIR_NAME);
    BAIL_ON_TDNF_ERROR(dwError);

    pDir = opendir(pszRepoCacheDir);
    if(pDir == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    while ((pEnt = readdir (pDir)) != NULL )
    {
        if (!strcmp(pEnt->d_name, ".") || !strcmp(pEnt->d_name, ".."))
        {
            continue;
        }

        dwError = TDNFAllocateStringPrintf(
                      &pszFilePath,
                      "%s/%s",
                      pszRepoCacheDir,
                      pEnt->d_name);
        BAIL_ON_TDNF_ERROR(dwError);
        if(pszFilePath)
        {
            if(unlink(pszFilePath))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
            TDNF_SAFE_FREE_MEMORY(pszFilePath);
            pszFilePath = NULL;
        }
        else
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if(rmdir(pszRepoCacheDir))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    if(pDir)
    {
        closedir(pDir);
    }
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
    char *pszRpmCacheArchDir = NULL;
    char *pszRpmCacheNoarchDir = NULL;
    char* pszFilePath = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;

    if (!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetRpmCacheDir(pTdnf, pszRepoId, &pszRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!IsNullOrEmptyString(pszRpmCacheDir))
    {
        dwError = TDNFAllocateStringPrintf(
                    &pszRpmCacheArchDir,
                    "%s/%s",
                    pszRpmCacheDir,
                    pTdnf->pConf->pszVarBaseArch);
        BAIL_ON_TDNF_ERROR(dwError);

        pDir = opendir(pszRpmCacheArchDir);
        if (pDir == NULL)
        {
            if (errno == ENOENT)
            {
                goto cleanup;
            }
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }

        while ((pEnt = readdir (pDir)) != NULL )
        {
            if (!strcmp(pEnt->d_name, ".") || !strcmp(pEnt->d_name, ".."))
            {
                continue;
            }

            dwError = TDNFAllocateStringPrintf(
                        &pszFilePath,
                        "%s/%s",
                        pszRpmCacheArchDir,
                        pEnt->d_name);
            BAIL_ON_TDNF_ERROR(dwError);
            if(pszFilePath)
            {
                if(unlink(pszFilePath))
                {
                    dwError = errno;
                    BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
                }
                TDNF_SAFE_FREE_MEMORY(pszFilePath);
                pszFilePath = NULL;
            }
            else
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        closedir(pDir);
        pDir = NULL;

        if(rmdir(pszRpmCacheArchDir))
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }

        dwError = TDNFAllocateStringPrintf(
                    &pszRpmCacheNoarchDir,
                    "%s/%s",
                    pszRpmCacheDir,
                    "noarch");
        BAIL_ON_TDNF_ERROR(dwError);

        pDir = opendir(pszRpmCacheNoarchDir);
        if(pDir == NULL)
        {
            if (errno == ENOENT)
            {
                goto cleanup;
            }
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }

        while ((pEnt = readdir (pDir)) != NULL )
        {
            if (!strcmp(pEnt->d_name, ".") || !strcmp(pEnt->d_name, ".."))
            {
                continue;
            }

            dwError = TDNFAllocateStringPrintf(
                        &pszFilePath,
                        "%s/%s",
                        pszRpmCacheNoarchDir,
                        pEnt->d_name);
            BAIL_ON_TDNF_ERROR(dwError);
            if(pszFilePath)
            {
                if(unlink(pszFilePath))
                {
                    dwError = errno;
                    BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
                }
                TDNF_SAFE_FREE_MEMORY(pszFilePath);
                pszFilePath = NULL;
            }
            else
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        closedir(pDir);
        pDir = NULL;

        if(rmdir(pszRpmCacheNoarchDir))
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

cleanup:
    if (pDir)
    {
        closedir(pDir);
    }
    if (!IsNullOrEmptyString(pszRpmCacheDir))
    {
        rmdir(pszRpmCacheDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheArchDir);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheNoarchDir);
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
    char* pszFilePath = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;

    if (IsNullOrEmptyString(pszTmpRepodataDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pDir = opendir(pszTmpRepodataDir);
    if(pDir == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    while ((pEnt = readdir (pDir)) != NULL )
    {
        if (!strcmp(pEnt->d_name, ".") || !strcmp(pEnt->d_name, ".."))
        {
            continue;
        }
        dwError = TDNFAllocateStringPrintf(
                    &pszFilePath,
                    "%s/%s",
                    pszTmpRepodataDir,
                    pEnt->d_name);
        if(pszFilePath)
        {
            if(unlink(pszFilePath))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
            TDNF_SAFE_FREE_MEMORY(pszFilePath);
            pszFilePath = NULL;
        }
        else
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if (rmdir(pszTmpRepodataDir))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
cleanup:
    if (pDir)
    {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
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

    dwError = TDNFAllocateStringPrintf(
                  &pszLastRefreshMarker,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_REPO_METADATA_MARKER);
    BAIL_ON_TDNF_ERROR(dwError);
    if (pszLastRefreshMarker)
    {
        if(unlink(pszLastRefreshMarker))
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
    char* pszFilePath = NULL;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;

    if(!pTdnf || !pTdnf->pConf || IsNullOrEmptyString(pszRepoId))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateStringPrintf(
                  &pszSolvCacheDir,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoId,
                  TDNF_SOLVCACHE_DIR_NAME);
    BAIL_ON_TDNF_ERROR(dwError);

    pDir = opendir(pszSolvCacheDir);
    if(pDir == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    while ((pEnt = readdir (pDir)) != NULL )
    {
        if (!strcmp(pEnt->d_name, ".") || !strcmp(pEnt->d_name, ".."))
        {
            continue;
        }

        dwError = TDNFAllocateStringPrintf(
                      &pszFilePath,
                      "%s/%s",
                      pszSolvCacheDir,
                      pEnt->d_name);
        BAIL_ON_TDNF_ERROR(dwError);
        if(pszFilePath)
        {
            if(unlink(pszFilePath))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
            TDNF_SAFE_FREE_MEMORY(pszFilePath);
            pszFilePath = NULL;
        }
        else
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if(rmdir(pszSolvCacheDir))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszSolvCacheDir);
    if(pDir)
    {
        closedir(pDir);
    }
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
    PTDNF_REPO_DATA_INTERNAL pRepo,
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
    PTDNF_REPO_DATA_INTERNAL pRepo,
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
    PTDNF_REPO_DATA_INTERNAL* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;

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
