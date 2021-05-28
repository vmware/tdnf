/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : utils.c
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
TDNFGetErrorString(
    uint32_t dwErrorCode,
    char** ppszError
    )
{
    uint32_t dwError = 0;
    char* pszError = NULL;
    char* pszSystemError = NULL;
    const char* pszCurlError = NULL;
    int i = 0;
    int nCount = 0;
    uint32_t dwActualError = 0;

    //Allow mapped error strings to override
    TDNF_ERROR_DESC arErrorDesc[] = TDNF_ERROR_TABLE;

    nCount = ARRAY_SIZE(arErrorDesc);

    for(i = 0; i < nCount; i++)
    {
        if (dwErrorCode == (uint32_t)arErrorDesc[i].nCode)
        {
            dwError = TDNFAllocateString(arErrorDesc[i].pszDesc, &pszError);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        }
    }

    //Get system error
    if(!pszError && TDNFIsSystemError(dwErrorCode))
    {
        dwActualError = TDNFGetSystemError(dwErrorCode);
        pszSystemError = strerror(dwActualError);
        if(pszSystemError)
        {
            dwError = TDNFAllocateString(pszSystemError, &pszError);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    //Get curl error
    if(!pszError && TDNFIsCurlError(dwErrorCode))
    {
        dwActualError = TDNFGetCurlError(dwErrorCode);
        pszCurlError = curl_easy_strerror(dwActualError);
        if(pszCurlError)
        {
            dwError = TDNFAllocateString(pszCurlError, &pszError);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    //If the above attempts did not yield an error string,
    //do default unknown error.
    if(!pszError)
    {
        dwError = TDNFAllocateString(TDNF_UNKNOWN_ERROR_STRING, &pszError);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszError = pszError;
cleanup:
    return dwError;

error:
    if(ppszError)
    {
        *ppszError = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszError);
    goto cleanup;
}

uint32_t
TDNFIsSystemError(
    uint32_t dwError
    )
{
    return dwError > ERROR_TDNF_SYSTEM_BASE;
}

uint32_t
TDNFIsCurlError(
    uint32_t dwError
    )
{
    return dwError >= ERROR_TDNF_CURL_BASE && dwError <= ERROR_TDNF_CURL_END;
}

uint32_t
TDNFGetSystemError(
    uint32_t dwError
    )
{
    uint32_t dwSystemError = 0;
    if(TDNFIsSystemError(dwError))
    {
        dwSystemError = dwError - ERROR_TDNF_SYSTEM_BASE;
    }
    return dwSystemError;
}

uint32_t
TDNFGetCurlError(
    uint32_t dwError
    )
{
    uint32_t dwCurlError = 0;
    if(TDNFIsCurlError(dwError))
    {
        dwCurlError = dwError - ERROR_TDNF_CURL_BASE;
    }
    return dwCurlError;
}

int
TDNFIsGlob(
    const char* pszString
    )
{
    int nResult = 0;
    while(*pszString)
    {
        char ch = *pszString;

        if(ch == '*' || ch == '?' || ch == '[')
        {
            nResult = 1;
            break;
        }

        pszString++;
    }
    return nResult;
}

uint32_t
TDNFUtilsMakeDir(
    const char* pszDir
    )
{
    uint32_t dwError = 0;

    if(IsNullOrEmptyString(pszDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(mkdir(pszDir, 0755))
    {
        if(errno != EEXIST)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFUtilsMakeDirs(
    const char* pszPath
    )
{
    uint32_t dwError = 0;
    char* pszTempPath = NULL;
    char* pszTemp = NULL;
    int nLength = 0;

    if(IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(access(pszPath, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        }
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    else
    {
        dwError = EEXIST;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPath, &pszTempPath);
    BAIL_ON_TDNF_ERROR(dwError);

    nLength = strlen(pszTempPath);
    if(pszTempPath[nLength - 1] == '/')
    {
        pszTempPath[nLength - 1] = '\0';
    }
    for(pszTemp = pszTempPath + 1; *pszTemp; pszTemp++)
    {
        if(*pszTemp == '/')
        {
            *pszTemp = '\0';
            dwError = TDNFUtilsMakeDir(pszTempPath);
            BAIL_ON_TDNF_ERROR(dwError);
            *pszTemp = '/';
        }
    }
    dwError = TDNFUtilsMakeDir(pszTempPath);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszTempPath);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFIsFileOrSymlink(
    const char* pszPath,
    int* pnPathIsFile
    )
{
    uint32_t dwError = 0;
    int nPathIsFile = 0;
    struct stat stStat = {0};

    if(!pnPathIsFile || IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(stat(pszPath, &stStat))
    {
        if (errno == ENOENT)
	{
            nPathIsFile = 0;
        }
        else
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }
    else
    {
        nPathIsFile = S_ISREG(stStat.st_mode) | S_ISLNK(stStat.st_mode);
    }

    *pnPathIsFile = nPathIsFile;
cleanup:
    return dwError;

error:
    if(pnPathIsFile)
    {
        *pnPathIsFile = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetFileSize(
    const char* pszPath,
    int *pnSize
    )
{
    uint32_t dwError = 0;
    struct stat stStat = {0};

    if(!pnSize || IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(stat(pszPath, &stStat))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    else
    {
        if (S_ISREG(stStat.st_mode))
        {
            *pnSize = stStat.st_size;
        }
    }
cleanup:
    return dwError;

error:
    if(pnSize)
    {
        *pnSize = 0;
    }
    goto cleanup;
}

uint32_t
TDNFIsDir(
    const char* pszPath,
    int* pnPathIsDir
    )
{
    uint32_t dwError = 0;
    int nPathIsDir = 0;
    struct stat stStat = {0};

    if(!pnPathIsDir || IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(stat(pszPath, &stStat))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    nPathIsDir = S_ISDIR(stStat.st_mode);

    *pnPathIsDir = nPathIsDir;
cleanup:
    return dwError;

error:
    if(pnPathIsDir)
    {
        *pnPathIsDir = 0;
    }
    goto cleanup;
}

//update time if file exists
//create if not
uint32_t
TDNFTouchFile(
    const char* pszFile
    )
{
    uint32_t dwError = 0;
    struct stat st = {0};
    int fd = -1;
    struct utimbuf times = {0};

    if(IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fd = creat(pszFile, S_IRUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
    {
        if(errno == EEXIST)
        {
            times.actime = st.st_atime;
            times.modtime = time(NULL);
            if(utime(pszFile, &times))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
        }
        else
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

cleanup:
    if(fd != -1)
    {
        close(fd);
    }
    return dwError;

error:
    goto cleanup;
}

//get package version using rpmlib
uint32_t
TDNFRawGetPackageVersion(
   const char* pszRootDir,
   const char* pszPkg,
   char** ppszVersion
   )
{
    uint32_t dwError = 0;
    const char* pszVersionTemp = NULL;
    char* pszVersion = NULL;
    rpmts pTS = NULL;
    Header pHeader = NULL;
    rpmdbMatchIterator pIter = NULL;

    if(IsNullOrEmptyString(pszRootDir) ||
       IsNullOrEmptyString(pszPkg) ||
       !ppszVersion)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pTS = rpmtsCreate();
    if(!pTS)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmtsSetRootDir (pTS, pszRootDir))
    {
        dwError = ERROR_TDNF_RPMTS_BAD_ROOT_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pIter = rpmtsInitIterator(pTS, RPMTAG_PROVIDES, pszPkg, 0);
    if(!pIter)
    {
        dwError = ERROR_TDNF_NO_DISTROVERPKG;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pHeader = rpmdbNextIterator(pIter);
    if(!pHeader)
    {
        dwError = ERROR_TDNF_DISTROVERPKG_READ;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pHeader = headerLink(pHeader);
    if(!pHeader)
    {
        dwError = ERROR_TDNF_DISTROVERPKG_READ;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszVersionTemp = headerGetString(pHeader, RPMTAG_VERSION);
    if(IsNullOrEmptyString(pszVersionTemp))
    {
        dwError = ERROR_TDNF_DISTROVERPKG_READ;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszVersionTemp, &pszVersion);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszVersion = pszVersion;
cleanup:
    if(pHeader)
    {
        headerFree(pHeader);
    }
    if(pIter)
    {
        rpmdbFreeIterator(pIter);
    }
    if(pTS)
    {
        rpmtsFree(pTS);
    }
    return dwError;

error:
    if(ppszVersion)
    {
        *ppszVersion = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszVersion);
    goto cleanup;
}

uint32_t
TDNFGetKernelArch(
   char** ppszArch
   )
{
    uint32_t dwError = 0;
    char* pszArch = NULL;
    struct utsname stUtsName;

    if(!ppszArch)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(uname(&stUtsName) != 0)
    {
        dwError = errno;
    }
    BAIL_ON_TDNF_SYSTEM_ERROR(dwError);

    dwError = TDNFAllocateString(stUtsName.machine, &pszArch);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszArch = pszArch;

cleanup:
    return dwError;

error:
    if(ppszArch)
    {
        *ppszArch = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszArch);
    goto cleanup;
}

uint32_t
TDNFParseMetadataExpire(
    const char* pszMetadataExpire,
    long* plMetadataExpire
    )
{
    uint32_t dwError = 0;
    long lMetadataExpire = -1;
    char* pszError = NULL;

    if(!plMetadataExpire || IsNullOrEmptyString(pszMetadataExpire))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!strcasecmp(TDNF_REPO_METADATA_EXPIRE_NEVER, pszMetadataExpire))
    {
        lMetadataExpire = -1;
    }
    else
    {
        lMetadataExpire = strtol(pszMetadataExpire, &pszError, 10);
        if(lMetadataExpire < 0)
        {
            lMetadataExpire = -1;
        }
        else if(lMetadataExpire > 0)
        {
            char chMultiplier = 's';
            int nMultiplier = 1;
            if(pszError && *pszError)
            {
                chMultiplier = *pszError;
            }
            switch(chMultiplier)
            {
                case 's': nMultiplier = 1; break;
                case 'm': nMultiplier = 60; break;
                case 'h': nMultiplier = 60*60; break;
                case 'd': nMultiplier = 60*60*24; break;
                default:
                    dwError = ERROR_TDNF_METADATA_EXPIRE_PARSE;
                    BAIL_ON_TDNF_ERROR(dwError);
            }
            lMetadataExpire *= nMultiplier;
        }
        else if(pszError && *pszError)
        {
            dwError = ERROR_TDNF_METADATA_EXPIRE_PARSE;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    *plMetadataExpire = lMetadataExpire;

cleanup:
    return dwError;

error:
    if(plMetadataExpire)
    {
        *plMetadataExpire = 0;
    }
    goto cleanup;
}

uint32_t
TDNFShouldSyncMetadata(
    const char* pszRepoDataFolder,
    long lMetadataExpire,
    int* pnShouldSync
    )
{
    uint32_t dwError = 0;
    int nShouldSync = 0;
    struct stat st = {0};
    time_t tCurrent = time(NULL);
    char* pszMarkerFile = NULL;

    if(!pnShouldSync || IsNullOrEmptyString(pszRepoDataFolder))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateStringPrintf(
                  &pszMarkerFile,
                  "%s/%s",
                  pszRepoDataFolder,
                  TDNF_REPO_METADATA_MARKER);
    BAIL_ON_TDNF_ERROR(dwError);

    //Look for the metadata marker file
    if(stat(pszMarkerFile, &st) == -1)
    {
        if(errno == ENOENT)
        {
            nShouldSync = 1;
        }
        else
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }
    else
    {
        if(difftime(tCurrent, st.st_ctime) > lMetadataExpire)
        {
            nShouldSync = 1;
        }
    }

    *pnShouldSync = nShouldSync;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszMarkerFile);
    return dwError;

error:
    if(pnShouldSync)
    {
        *pnShouldSync = 0;
    }
    goto cleanup;
}

uint32_t
TDNFAppendPath(
    const char *pszBase,
    const char *pszPart,
    char **ppszPath
    )
{
    uint32_t dwError = 0;
    char *pszPath = NULL;
    int nLength = 0;

    if(IsNullOrEmptyString(pszBase) ||
       IsNullOrEmptyString(pszPart) ||
       !ppszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  strlen(pszBase) + strlen(pszPart) + 2,
                  (void **)&pszPath);
    BAIL_ON_TDNF_ERROR(dwError);

    nLength = strlen(pszBase);
    if(pszBase[nLength - 1] == '/')
    {
        --nLength;
    }

    strncpy(pszPath, pszBase, nLength);
    if(pszPart[0] != '/')
    {
        strcat(pszPath, "/");
    }
    strcat(pszPath, pszPart);

    *ppszPath = pszPath;
cleanup:
    return dwError;

error:
    if(ppszPath)
    {
        *ppszPath = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszPath);
    goto cleanup;
}

uint32_t
TDNFGetCmdOpt(
    PTDNF pTdnf,
    TDNF_CMDOPT_TYPE optType,
    PTDNF_CMD_OPT *ppOpt
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pOpt = NULL;
    int nFound = 0;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pOpt = pTdnf->pArgs->pSetOpt;
         pOpt;
         pOpt = pOpt->pNext)
    {
        if (pOpt->nType == optType)
        {
            nFound = 1;
            break;
        }
    }

    if (!nFound)
    {
        dwError = ERROR_TDNF_OPT_NOT_FOUND;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppOpt = pOpt;

error:
    return dwError;
}

