
/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
SolvLoadRepomd(
    Repo* pRepo,
    const char* pszRepomd
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszRepomd))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = fopen(pszRepomd, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_repomdxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdPrimary(
    Repo* pRepo,
    const char* pszPrimary
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszPrimary))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = solv_xfopen(pszPrimary, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, 0, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvLoadRepomdFilelists(
    Repo* pRepo,
    const char* pszFilelists
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if(!pRepo || IsNullOrEmptyString(pszFilelists))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszFilelists, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, "FL", REPO_EXTEND_SOLVABLES))
    {
        dwError = ERROR_TDNF_SOLV_FAILED;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
        fclose(fp);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdUpdateinfo(
    Repo* pRepo,
    const char* pszUpdateinfo
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszUpdateinfo))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszUpdateinfo, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_updateinfoxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdOther(
    Repo* pRepo,
    const char* pszOther
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszOther))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = solv_xfopen(pszOther, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, 0, REPO_EXTEND_SOLVABLES))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvReadYumRepo(
    Repo *pRepo,
    const char *pszRepoName,
    const char *pszRepomd,
    const char *pszPrimary,
    const char *pszFilelists,
    const char *pszUpdateinfo,
    const char *pszOther
    )
{
    uint32_t dwError = 0;
    if(!pRepo || !pszRepoName || !pszRepomd || !pszPrimary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = SolvLoadRepomd(pRepo, pszRepomd);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);


    dwError = SolvLoadRepomdPrimary(pRepo, pszPrimary);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if(pszFilelists)
    {
        dwError = SolvLoadRepomdFilelists(pRepo, pszFilelists);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszUpdateinfo)
    {
        dwError = SolvLoadRepomdUpdateinfo(pRepo, pszUpdateinfo);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszOther)
    {
        dwError = SolvLoadRepomdOther(pRepo, pszOther);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }


cleanup:

    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCountPackages(
    PSolvSack pSack,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    Pool* pool = 0;
    Id p = 0;
    if(!pSack || !pSack->pPool || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pool = pSack->pPool;
    FOR_POOL_SOLVABLES(p)
    {
        dwCount++;
    }
    *pdwCount = dwCount;
cleanup:
    return dwError;
error:
    goto cleanup;

}

static
uint32_t
readRpmsFromDir(
    Repo *pRepo,
    const char *pszDir
)
{
    uint32_t dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    char *pszPath = NULL;

    pDir = opendir(pszDir);
    if(pDir == NULL) {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    while ((pEnt = readdir (pDir)) != NULL ) {
        int isDir;

        if (pEnt->d_name[0] == '.') {
            /* skip '.', '..', but also any dir name starting with '.' */
            continue;
        }

        dwError = TDNFJoinPath(
                      &pszPath,
                      pszDir,
                      pEnt->d_name,
                      NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFIsDir(pszPath, &isDir);
        BAIL_ON_TDNF_ERROR(dwError);

        if (isDir) {
            dwError = readRpmsFromDir(pRepo, pszPath);
            BAIL_ON_TDNF_ERROR(dwError);
        } else if (strcmp(&pEnt->d_name[strlen(pEnt->d_name)-4], ".rpm") == 0) {
            if(!repo_add_rpm(pRepo,
                             pszPath,
                             REPO_REUSE_REPODATA|REPO_NO_INTERNALIZE)) {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        TDNF_SAFE_FREE_MEMORY(pszPath);
    }
cleanup:
    if(pDir) {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszPath);

    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvReadRpmsFromDirectory(
    Repo *pRepo,
    const char *pszDir
    )
{
    uint32_t dwError = 0;

    if(!pRepo || IsNullOrEmptyString(pszDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = readRpmsFromDir(pRepo, pszDir);
    BAIL_ON_TDNF_ERROR(dwError);

    repo_internalize(pRepo);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvReadInstalledRpms(
    Repo* pRepo,
    const char *pszCacheFileName
    )
{
    uint32_t dwError = 0;
    FILE *pCacheFile = NULL;
    int  dwFlags = 0;

    if(!pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszCacheFileName && access(pszCacheFileName, F_OK) == 0)
    {
        /* coverity[toctou] */
        pCacheFile = fopen(pszCacheFileName, "r");

        if(!pCacheFile)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

    dwFlags = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    dwError = repo_add_rpmdb_reffp(pRepo, pCacheFile, dwFlags);

    if (dwError)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (pCacheFile)
        fclose(pCacheFile);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCalculateCookieForFile(
    char *pszFilePath,
    unsigned char *pszCookie
    )
{
    FILE *fp = NULL;
    int32_t nLen = 0;
    uint32_t dwError = 0;
    Chksum *pChkSum = NULL;
    char buf[BUFSIZ] = {0};

    if (!pszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = fopen(pszFilePath, "r");
    if (!fp)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pChkSum = solv_chksum_create(REPOKEY_TYPE_SHA256);
    if (!pChkSum)
    {
        dwError = ERROR_TDNF_SOLV_CHKSUM;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    solv_chksum_add(pChkSum, SOLV_COOKIE_IDENT, strlen(SOLV_COOKIE_IDENT));

    while ((nLen = fread(buf, 1, sizeof(buf) - 1, fp)) > 0)
    {
          solv_chksum_add(pChkSum, buf, nLen);
          memset(buf, 0, sizeof(buf));
    }
    solv_chksum_free(pChkSum, pszCookie);

cleanup:
    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:
    goto cleanup;
}

/* Create a name for the repo cache path based on repo name and
   a hash of the url.
*/
uint32_t
SolvCreateRepoCacheName(
    const char *pszName,
    const char *pszUrl,
    char **ppszCacheName
    )
{
    uint32_t dwError = 0;
    Chksum *pChkSum = NULL;
    unsigned char pCookie[SOLV_COOKIE_LEN] = {0};
    char pszCookie[9] = {0};
    char *pszCacheName = NULL;

    if (!pszName || !pszUrl || !ppszCacheName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pChkSum = solv_chksum_create(REPOKEY_TYPE_SHA256);
    if (!pChkSum)
    {
        dwError = ERROR_TDNF_SOLV_CHKSUM;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    solv_chksum_add(pChkSum, pszUrl, strlen(pszUrl));
    solv_chksum_free(pChkSum, pCookie);

    snprintf(pszCookie, sizeof(pszCookie), "%.2x%.2x%.2x%.2x",
             pCookie[0], pCookie[1], pCookie[2], pCookie[3]);

    dwError = TDNFAllocateStringPrintf(&pszCacheName, "%s-%s", pszName, pszCookie);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    *ppszCacheName = pszCacheName;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszCacheName);
    goto cleanup;
}

uint32_t
SolvGetMetaDataCachePath(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    char** ppszCachePath
    )
{
    char *pszCachePath = NULL;
    uint32_t dwError = 0;
    Repo *pRepo = NULL;

    if (!pSolvRepoInfo || !pSolvRepoInfo->pRepo || !ppszCachePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pRepo = pSolvRepoInfo->pRepo;
    if (!IsNullOrEmptyString(pRepo->name))
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszCachePath,
                      "%s/%s/%s.solv",
                      pSolvRepoInfo->pszRepoCacheDir,
                      TDNF_SOLVCACHE_DIR_NAME,
                      pRepo->name);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszCachePath = pszCachePath;
cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszCachePath);
    goto cleanup;
}

uint32_t
SolvAddSolvMetaData(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    char *pszTempSolvFile
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    FILE *fp = NULL;
    int i = 0;

    if (!pSolvRepoInfo || !pSolvRepoInfo->pRepo || !pszTempSolvFile)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pRepo = pSolvRepoInfo->pRepo;
    if (!pRepo->pool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    for (i = pRepo->start; i < pRepo->end; i++)
    {
         if (pRepo->pool->solvables[i].repo != pRepo)
         {
             break;
         }
    }
    if (i < pRepo->end)
    {
        goto cleanup;
    }
    fp = fopen (pszTempSolvFile, "r");
    if (fp == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    repo_empty(pRepo, 1);
    if (repo_add_solv(pRepo, fp, SOLV_ADD_NO_STUBS))
    {
        dwError = ERROR_TDNF_ADD_SOLV;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (fp != NULL)
    {
        fclose(fp);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvUseMetaDataCache(
    PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    int       *nUseMetaDataCache
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    Repo *pRepo = NULL;
    unsigned char *pszCookie = NULL;
    unsigned char pszTempCookie[32];
    char *pszCacheFilePath = NULL;

    if (!pSack || !pSolvRepoInfo || !pSolvRepoInfo->pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pRepo = pSolvRepoInfo->pRepo;
    pszCookie = pSolvRepoInfo->nCookieSet ? pSolvRepoInfo->cookie : NULL;

    dwError = SolvGetMetaDataCachePath(pSolvRepoInfo, &pszCacheFilePath);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (IsNullOrEmptyString(pszCacheFilePath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = fopen(pszCacheFilePath, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_CACHE_NOT_CREATED;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    // Reading the cookie from cached Solv File
    if (fseek (fp, -sizeof(pszTempCookie), SEEK_END) || fread (pszTempCookie, sizeof(pszTempCookie), 1, fp) != 1)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    // compare the calculated cookie with the one read from Solv file
    if (pszCookie && memcmp (pszCookie, pszTempCookie, sizeof(pszTempCookie)) != 0)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    rewind(fp);
    if (repo_add_solv(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_ADD_SOLV;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    *nUseMetaDataCache = 1;

cleanup:
    if (fp != NULL)
    {
       fclose(fp);
    }
    TDNF_SAFE_FREE_MEMORY(pszCacheFilePath);
    return dwError;
error:
    if (dwError == ERROR_TDNF_SOLV_CACHE_NOT_CREATED)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
SolvCreateMetaDataCache(
    PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    FILE *fp = NULL;
    int fd = 0;
    char *pszSolvCacheDir = NULL;
    char *pszTempSolvFile = NULL;
    char *pszCacheFilePath = NULL;
    mode_t mask = 0;

    if (!pSack || !pSolvRepoInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pRepo = pSolvRepoInfo->pRepo;
    dwError = TDNFJoinPath(
                  &pszSolvCacheDir,
                  pSolvRepoInfo->pszRepoCacheDir,
                  TDNF_SOLVCACHE_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (access(pszSolvCacheDir, W_OK | X_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        }
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszSolvCacheDir);
        if (dwError == ERROR_TDNF_ALREADY_EXISTS)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszTempSolvFile = solv_dupjoin(pszSolvCacheDir, "/", ".newsolv-XXXXXX");
    mask = umask(S_IRUSR | S_IWUSR | S_IRWXG);
    umask(mask);
    fd = mkstemp(pszTempSolvFile);
    if (fd < 0)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fchmod(fd, 0444);
    fp = fdopen(fd, "w");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_write(pRepo, fp))
    {
        dwError = ERROR_TDNF_REPO_WRITE;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if (pSolvRepoInfo->nCookieSet)
    {
        if (fwrite(pSolvRepoInfo->cookie, SOLV_COOKIE_LEN, 1, fp) != 1)
        {
            dwError = ERROR_TDNF_SOLV_IO;
            BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
        }
    }

    if (fclose(fp))
    {
        fp = NULL;/* so that error branch will not attempt to close again */
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = NULL;
    dwError = SolvAddSolvMetaData(pSolvRepoInfo, pszTempSolvFile);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    dwError = SolvGetMetaDataCachePath(pSolvRepoInfo, &pszCacheFilePath);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (IsNullOrEmptyString(pszCacheFilePath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if (rename(pszTempSolvFile, pszCacheFilePath) == -1)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    unlink(pszTempSolvFile);
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszTempSolvFile);
    TDNF_SAFE_FREE_MEMORY(pszSolvCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszCacheFilePath);
    return dwError;
error:
    if (fp != NULL)
    {
        fclose(fp);
        unlink(pszTempSolvFile);
    }
    else if (fd > 0)
    {
        close(fd);
        unlink(pszTempSolvFile);
    }
    goto cleanup;
}
