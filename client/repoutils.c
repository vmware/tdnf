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
    const char* pszRepo,
    char* pszBaseUrl
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA_INTERNAL pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszRepo) || !pszBaseUrl)
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
    TDNF_SAFE_FREE_MEMORY(pRepos->pszBaseUrl);
    dwError = TDNFAllocateString(pszBaseUrl, &pRepos->pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);


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

    dwError = TDNFAllocateString(pRepos->pszBaseUrl, &pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

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
    const char* pszTmpRepodataDir,
    const char* pszTmpRepoMDFile
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszTmpRepodataDir) || IsNullOrEmptyString(pszTmpRepoMDFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (unlink(pszTmpRepoMDFile))
    {
        if (errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if (rmdir(pszTmpRepodataDir))
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
