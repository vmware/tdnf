/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

hash_op hash_ops[TDNF_HASH_SENTINEL] =
    {
       [TDNF_HASH_MD5]    = {"md5", MD5_DIGEST_LENGTH},
       [TDNF_HASH_SHA1]   = {"sha1", SHA_DIGEST_LENGTH},
       [TDNF_HASH_SHA256] = {"sha256", SHA256_DIGEST_LENGTH},
       [TDNF_HASH_SHA512] = {"sha512", SHA512_DIGEST_LENGTH},
    };

hash_type hashType[] =
    {
        {"md5", TDNF_HASH_MD5},
        {"sha1", TDNF_HASH_SHA1},
        {"sha-1", TDNF_HASH_SHA1},
        {"sha256", TDNF_HASH_SHA256},
        {"sha-256", TDNF_HASH_SHA256},
        {"sha512", TDNF_HASH_SHA512},
        {"sha-512", TDNF_HASH_SHA512}
    };

uint32_t
TDNFFileReadAllText(
    const char *pszFileName,
    char **ppszText,
    int *pnLength
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    char *pszText = NULL;
    int nLength = 0;
    int nBytesRead = 0;

    if(!pszFileName || !ppszText)
    {
        dwError = EINVAL;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    fp = fopen(pszFileName, "r");
    if(!fp)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fseek(fp, 0, SEEK_END);
    nLength = ftell(fp);

    if (nLength < 0) {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, nLength + 1, (void **)&pszText);
    BAIL_ON_TDNF_ERROR(dwError);

    if(fseek(fp, 0, SEEK_SET))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    nBytesRead = fread(pszText, 1, nLength, fp);
    if(nBytesRead != nLength)
    {
        dwError = EBADFD;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    pszText[nLength] = 0;

    *ppszText = pszText;
    if (pnLength != NULL) {
        *pnLength = nLength;
    }
cleanup:
    if(fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    if(ppszText)
    {
        *ppszText = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszText);
    goto cleanup;
}

const char *
TDNFLeftTrim(
    const char *pszStr
    )
{
    if(!pszStr) return NULL;
    while(isspace(*pszStr)) ++pszStr;
    return pszStr;
}

const char *
TDNFRightTrim(
    const char *pszStart,
    const char *pszEnd
    )
{
    if(!pszStart || !pszEnd) return NULL;
    while(pszEnd > pszStart && isspace(*pszEnd)) pszEnd--;
    return pszEnd;
}

uint32_t
TDNFCreateAndWriteToFile(
    const char *pszFile,
    const char *data
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;

    if (IsNullOrEmptyString(pszFile) || IsNullOrEmptyString(data))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = fopen(pszFile, "w");
    if (!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }
    fputs(data, fp);
    fclose(fp);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFUtilsFormatSize(
    uint64_t unSize,
    char** ppszFormattedSize
    )
{
    uint32_t dwError = 0;
    char* pszFormattedSize = NULL;
    const char* pszSizes = "bkMG";
    double dSize = unSize;

    int nIndex = 0;
    int nLimit = strlen(pszSizes);
    double dKiloBytes = 1024.0;
    int nMaxSize = 512;
    int nWritten;

    if(!ppszFormattedSize)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    while(nIndex < nLimit && dSize > dKiloBytes)
    {
        dSize /= dKiloBytes;
        nIndex++;
    }

    dwError = TDNFAllocateMemory(1, nMaxSize, (void**)&pszFormattedSize);
    BAIL_ON_TDNF_ERROR(dwError);

    nWritten = snprintf(pszFormattedSize, nMaxSize, "%6.2f%c", dSize, pszSizes[nIndex]);
    if (nWritten < 0 || nWritten >= nMaxSize)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFormattedSize = pszFormattedSize;

cleanup:
    return dwError;

error:
    if(ppszFormattedSize)
    {
        *ppszFormattedSize = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszFormattedSize);
    goto cleanup;
}

void
TDNFFreePackageInfo(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    while(pPkgInfo)
    {
        PTDNF_PKG_INFO pPkgInfoTemp = pPkgInfo;
        pPkgInfo = pPkgInfo->pNext;

        TDNFFreePackageInfoContents(pPkgInfoTemp);
        TDNFFreeMemory(pPkgInfoTemp);
    }
}

void
TDNFFreePackageInfoArray(
    PTDNF_PKG_INFO pPkgInfoArray,
    uint32_t unLength
    )
{
  if (!pPkgInfoArray) {
      return;
    }

    while ((int32_t)--unLength >= 0) {
      TDNFFreePackageInfoContents(&pPkgInfoArray[unLength]);
    }

    TDNF_SAFE_FREE_MEMORY(pPkgInfoArray);
}

void
TDNFFreeChangeLogEntry(
    PTDNF_PKG_CHANGELOG_ENTRY pEntry
)
{
    if (pEntry)
    {
        TDNF_SAFE_FREE_MEMORY(pEntry->pszAuthor);
        TDNF_SAFE_FREE_MEMORY(pEntry->pszText);
        TDNF_SAFE_FREE_MEMORY(pEntry);
    }
}

void
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    PTDNF_PKG_CHANGELOG_ENTRY pEntry, pEntryNext;

    if(pPkgInfo)
    {
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszName);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRepoName);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszVersion);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszArch);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszEVR);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszSummary);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszURL);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszLicense);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszDescription);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszFormattedSize);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszFormattedDownloadSize);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRelease);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszLocation);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pbChecksum);

        if(pPkgInfo->pppszDependencies)
        {
            int depKey;
            for (depKey = 0; depKey < REPOQUERY_DEP_KEY_COUNT; depKey++)
            {
                TDNF_SAFE_FREE_STRINGARRAY(pPkgInfo->pppszDependencies[depKey]);
            }
            TDNFFreeMemory(pPkgInfo->pppszDependencies);
        }

        TDNF_SAFE_FREE_STRINGARRAY(pPkgInfo->ppszFileList);
        for (pEntry = pPkgInfo->pChangeLogEntries;
             pEntry;
             pEntry = pEntryNext)
        {
            pEntryNext = pEntry->pNext;
            TDNFFreeChangeLogEntry(pEntry);
        }
    }
}

void
TDNFFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    if (!pSolvedPkgInfo)
    {
        return;
    }

    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsNotAvailable);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsExisting);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToInstall);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToUpgrade);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToDowngrade);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToRemove);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsUnNeeded);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsToReinstall);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsObsoleted);
    TDNF_SAFE_FREE_PKGINFO(pSolvedPkgInfo->pPkgsRemovedByDowngrade);

    TDNF_SAFE_FREE_STRINGARRAY(pSolvedPkgInfo->ppszPkgsNotResolved);
    TDNF_SAFE_FREE_STRINGARRAY(pSolvedPkgInfo->ppszPkgsUserInstall);

    TDNF_SAFE_FREE_MEMORY(pSolvedPkgInfo);
}

void
TDNFFreeUpdateInfoSummary(
    PTDNF_UPDATEINFO_SUMMARY pSummary
    )
{
    if(pSummary)
    {
        TDNFFreeMemory(pSummary);
    }
}

static void
TDNFFreeUpdateInfoReferences(
    PTDNF_UPDATEINFO_REF pRef
    )
{
    if(pRef)
    {
        TDNF_SAFE_FREE_MEMORY(pRef->pszID);
        TDNF_SAFE_FREE_MEMORY(pRef->pszLink);
        TDNF_SAFE_FREE_MEMORY(pRef->pszTitle);
        TDNF_SAFE_FREE_MEMORY(pRef->pszType);
    }
}

void
TDNFFreeUpdateInfoPackages(
    PTDNF_UPDATEINFO_PKG pPkgs
    )
{
    PTDNF_UPDATEINFO_PKG pTemp = NULL;
    while(pPkgs)
    {
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszFileName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszEVR);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszArch);

        pTemp = pPkgs;
        pPkgs = pPkgs->pNext;

        TDNF_SAFE_FREE_MEMORY(pTemp);
    }
}

void
TDNFFreeUpdateInfo(
    PTDNF_UPDATEINFO pUpdateInfo
    )
{
    if(pUpdateInfo)
    {
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszID);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDate);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDescription);

        TDNFFreeUpdateInfoReferences(pUpdateInfo->pReferences);
        TDNFFreeUpdateInfoPackages(pUpdateInfo->pPackages);
        TDNFFreeMemory(pUpdateInfo);
    }
}

void
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    )
{
    PTDNF_CMD_OPT pCmdOptTemp = NULL;
    while(pCmdOpt)
    {
        TDNF_SAFE_FREE_MEMORY(pCmdOpt->pszOptName);
        TDNF_SAFE_FREE_MEMORY(pCmdOpt->pszOptValue);
        pCmdOptTemp = pCmdOpt->pNext;
        TDNF_SAFE_FREE_MEMORY(pCmdOpt);
        pCmdOpt = pCmdOptTemp;
    }
}

void
TDNFFreeCmdArgs(
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    if (!pCmdArgs)
    {
        return;
    }

    for(int nIndex = 0; nIndex < pCmdArgs->nCmdCount; ++nIndex)
    {
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds[nIndex]);
    }

    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszArch);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszDownloadDir);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszInstallRoot);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszConfFile);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszReleaseVer);

    destroy_cnftree(pCmdArgs->cn_setopts);
    destroy_cnftree(pCmdArgs->cn_repoopts);
    TDNF_SAFE_FREE_MEMORY(pCmdArgs);
}

void
TDNFFreeRepos(
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
        TDNF_SAFE_FREE_MEMORY(pRepo->pszSnapshotUrl);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszSnapshotFile);
        TDNF_SAFE_FREE_STRINGARRAY(pRepo->ppszUrlGPGKeys);

        pRepos = pRepo->pNext;
        TDNF_SAFE_FREE_MEMORY(pRepo);
    }
}

uint32_t
TDNFYesOrNo(
    PTDNF_CMD_ARGS pArgs,
    const char *pszQuestion,
    int *pAnswer
    )
{
    uint32_t dwError = 0;
    int32_t opt = 0;

    if(!pArgs || !pszQuestion || !pAnswer)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }
    *pAnswer = 0;

    if(!pArgs->nAssumeYes && !pArgs->nAssumeNo)
    {
        while(1) {
            char buf[256] = {0};
            const char *ret;

            pr_crit("%s", pszQuestion);

            ret = fgets(buf, sizeof(buf)-1, stdin);
            if (ret != buf || buf[0] == 0) {
                /* should not happen */
                dwError = ERROR_TDNF_INVALID_INPUT;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            buf[strlen(buf)-1] = 0;
            if (strcasecmp(buf, "yes") == 0 || strcasecmp(buf, "y") == 0 ||
                    strcasecmp(buf, "n") == 0 || strcasecmp(buf, "no") == 0 ||
                    buf[0] == 0) {
                opt = tolower(buf[0]);
                break;
            }
        }
    }

    if(pArgs->nAssumeYes || opt == 'y')
    {
        *pAnswer = 1;
    }
error:
    return dwError;
}

uint32_t
TDNFUriIsRemote(
    const char* pszKeyUrl,
    int *pnRemote
)
{
    uint32_t dwError = 0;
    const char *szRemotes[] = {"http://", "https://", "ftp://",
                               "ftps://", NULL};
    int i = 0;

    if(!pnRemote || IsNullOrEmptyString(pszKeyUrl))
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    *pnRemote = 0;
    for(i = 0; szRemotes[i]; i++) {
        if (strncasecmp(pszKeyUrl, szRemotes[i], strlen(szRemotes[i])) == 0) {
            *pnRemote = 1;
            break;
        }
    }
    if (szRemotes[i] == NULL && strncasecmp(pszKeyUrl, "file://", 7) != 0) {
       dwError = ERROR_TDNF_URL_INVALID;
       BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPathFromUri(
    const char* pszKeyUrl,
    char** ppszPath
)
{
    uint32_t dwError = 0;
    const char* pszPath = NULL;
    char *pszPathTmp = NULL;
    size_t nOffset;
    const char *szProtocols[] = {"http://", "https://", "ftp://",
                                "ftps://", "file://", NULL};
    int i = 0;

    if(IsNullOrEmptyString(pszKeyUrl) || !ppszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(i = 0; szProtocols[i]; i++) {
        if (strncasecmp(pszKeyUrl, szProtocols[i], strlen(szProtocols[i])) == 0) {
            nOffset = strlen(szProtocols[i]);
            break;
        }
    }
    if (szProtocols[i] == NULL) {
       dwError = ERROR_TDNF_URL_INVALID;
       BAIL_ON_TDNF_ERROR(dwError);
    }

    pszPath = pszKeyUrl + nOffset;

    if(!pszPath || *pszPath == '\0')
    {
        dwError = ERROR_TDNF_URL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (strchr (pszPath, '#') != NULL)
    {
        dwError = ERROR_TDNF_URL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(*pszPath != '/')
    {
        //skip hostname in the uri.
        pszPath = strchr (pszPath, '/');
        if(pszPath == NULL)
        {
            dwError = ERROR_TDNF_URL_INVALID;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    dwError = TDNFAllocateString(pszPath, &pszPathTmp);
    BAIL_ON_TDNF_ERROR(dwError);
    *ppszPath = pszPathTmp;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszPathTmp);
    if(ppszPath)
    {
        *ppszPath = NULL;
    }
    goto cleanup;
}


uint32_t
TDNFNormalizePath(
    const char* pszPath,
    char** ppszNormalPath)
{
    uint32_t dwError = 0;
    char* pszNormalPath = NULL;
    char* pszRealPath = NULL;
    const char* p = pszPath;
    char* q;

    if (IsNullOrEmptyString(pszPath) || !ppszNormalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* ensure an absolute path */
    if (pszPath[0] != '/')
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, strlen(pszPath) + 1, (void **)&pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    q = pszNormalPath;

    while(*p)
    {
        /* double slashes */
        if (*p == '/' && p[1] == '/')
        {
            p++;
            continue;
        }
        /* single dots */
        if (*p == '/' && p[1] == '.' &&
            (p[2] == '/' || p[2] == 0))
        {
            p += 2;
            continue;
        }
        /* double dots */
        if (*p == '/' && p[1] == '.' && p[2] == '.' &&
            (p[3] == '/' || p[3] == 0))
        {
            /* breaking out */
            if (q == pszNormalPath)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            p += 3;
            /* erase last directory */
            while (q > pszNormalPath && *q != '/')
            {
                q--;
            }
            continue;
        }
        if (*p == '/')
        {
            *q = 0;
            /* use realpath() to resolve symlinks */
            pszRealPath = realpath(pszNormalPath, NULL);
            if (pszRealPath != NULL)
            {
                /* if real path is different, copy it, making
                 * sure we still have enough space, and reposition dest pointer */
                if (strcmp(pszRealPath, pszNormalPath))
                {
                    int rlen = strlen(pszRealPath);
                    TDNF_SAFE_FREE_MEMORY(pszNormalPath);

                    dwError = TDNFAllocateMemory(1, rlen + strlen(p) + 1,
                                                 (void **)&pszNormalPath);
                    BAIL_ON_TDNF_ERROR(dwError);

                    strcpy(pszNormalPath, pszRealPath);
                    q = pszNormalPath + rlen;
                }
                TDNF_SAFE_FREE_MEMORY(pszRealPath);
            }
            /* it's okay if path doesn't exist, bail on other errors */
            else if (errno != ENOENT)
            {
                dwError = ERROR_TDNF_SYSTEM_BASE + errno;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            /* skip over last slash in path */
            if (p[1] == 0)
            {
                p++;
                continue;
            }
        }
        *q++ = *p++;
    }
    *q = 0;

    /* an empty path should evaluate to a slash
     * like realpath() does */
    if (pszNormalPath[0] == 0)
    {
        TDNF_SAFE_FREE_MEMORY(pszNormalPath);
        pszNormalPath = strdup("/");
    }

    /* check real path for leaf node, which wasn't checked above */
    pszRealPath = realpath(pszNormalPath, NULL);
    if (pszRealPath != NULL)
    {
        TDNF_SAFE_FREE_MEMORY(pszNormalPath);
        pszNormalPath = pszRealPath;
    }
    else if (errno != ENOENT)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszNormalPath = pszNormalPath;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszNormalPath);
    TDNF_SAFE_FREE_MEMORY(pszRealPath);
    if(ppszNormalPath)
    {
        *ppszNormalPath = NULL;
    }
    goto cleanup;
}

static int
_rm_file(const char *path, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    UNUSED(sbuf);
    UNUSED(type);
    UNUSED(ftwb);

    if(remove(path) < 0)
    {
        pr_crit("unable to remove %s: %s\n", path, strerror(errno));
    }
    return 0;
}

uint32_t
TDNFRecursivelyRemoveDir(const char *pszPath)
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nftw(pszPath, _rm_file, 10, FTW_DEPTH|FTW_PHYS) < 0)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

/* search pszSearch in the string ppszList, result will be in pRet */
uint32_t
TDNFStringMatchesOneOf(const char *pszSearch, char **ppszList, int *pRet)
{
    int i;
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszSearch) || !ppszList || !pRet)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pRet = 0;
    for(i = 0; ppszList[i]; i++)
    {
        if (strcmp(pszSearch, ppszList[i]) == 0)
        {
            *pRet = 1;
            goto cleanup;
        }
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFJoinPath(char **ppszPath, ...)
{
    uint32_t dwError = 0;
    va_list ap;
    char *pszNode = NULL;
    int i, nCount = 0;
    char *pszTmp, *pszNodeTmp;
    char *pszNodeCopy = NULL;
    char *pszResult = NULL;
    int nLength = 0;

    if (!ppszPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    va_start(ap, ppszPath);
    for(pszNode = va_arg(ap, char *); pszNode; pszNode = va_arg(ap, char *))
    {
        nCount++;
    }
    va_end(ap);

    va_start(ap, ppszPath);
    for(pszNode = va_arg(ap, char *), i = 0; pszNode; pszNode = va_arg(ap, char *), i++)
    {
        int nLengthTmp = 0;
        dwError = TDNFAllocateString(pszNode, &pszNodeCopy);
        BAIL_ON_TDNF_ERROR(dwError);
        pszNodeTmp = pszNodeCopy;
        /* if the first node is an absolute path, the result should be absolute -
         * safe this by initializing with a '/' if absolute, otherwise with an empty string
         * before stripping all leading slashes */
        if (i == 0)
        {
            if (*pszNodeTmp == '/')
            {
                dwError = TDNFAllocateString("/", &pszResult);
                nLength++;
            }
            else
            {
                dwError = TDNFAllocateString("", &pszResult);
            }
            BAIL_ON_TDNF_ERROR(dwError);
        }
        /* now strip leading slashes */
        while(*pszNodeTmp == '/') pszNodeTmp++;

        /* strip trailing slashes */
        nLengthTmp = strlen(pszNodeTmp);
        pszTmp = pszNodeTmp + nLengthTmp - 1;
        while(pszTmp >= pszNodeTmp && *pszTmp == '/')
        {
            *pszTmp = 0;
            pszTmp--;
        }
        nLength += nLengthTmp + 2;

        dwError = TDNFReAllocateMemory(nLength, (void **)&pszResult);
        BAIL_ON_TDNF_ERROR(dwError);

        strcat(pszResult, pszNodeTmp);
        /* put new slashes between nodes, except for the end */
        if (i != nCount-1)
        {
            strcat(pszResult, "/");
        }

        TDNF_SAFE_FREE_MEMORY(pszNodeCopy);
    }

    *ppszPath = pszResult;
cleanup:
    va_end(ap);
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszResult);
    TDNF_SAFE_FREE_MEMORY(pszNodeCopy);
    goto cleanup;
}

/* read all lines in file pszFile and store in string array
 * pointed to by pppszArray, one entry for each line */
uint32_t
TDNFReadFileToStringArray(
    const char *pszFile,
    char ***pppszArray
    )
{
    uint32_t dwError = 0;
    int nLength = 0;
    char *pszText = NULL;
    char **ppszArray = NULL;

    if (!pszFile || !pppszArray)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFileReadAllText(pszFile, &pszText, &nLength);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSplitStringToArray(pszText, (char *)"\n", &ppszArray);
    BAIL_ON_TDNF_ERROR(dwError);

    *pppszArray = ppszArray;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszText);
    return dwError;
error:
    TDNF_SAFE_FREE_STRINGARRAY(ppszArray);
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

uint32_t
TDNFDirName(
    const char *pszPath,
    char **ppszDirName
)
{
    uint32_t dwError = 0;
    char *pszDirName = NULL;
    char *pszPathCopy = NULL;

    if(!pszPath || IsNullOrEmptyString(pszPath) || !ppszDirName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPath, &pszPathCopy);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(dirname(pszPathCopy), &pszDirName);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszDirName = pszDirName;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPathCopy);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszDirName);
    goto cleanup;
}

int32_t strtoi(const char *ptr)
{
    char *p = NULL;
    long int tmp = 0;

    tmp = strtol(ptr, &p, 10);

    if (*p || tmp > INT_MAX || tmp < INT_MIN)
    {
        pr_crit("WARNING: invalid arg to %s: '%s'\n", __func__, ptr);
        return 0;
    }

    return (int32_t) tmp;
}

int isTrue(const char *str)
{
    if (!strcasecmp(str, "false"))
        return 0;

    return !strcasecmp(str, "true") || strtoi(str);
}

uint32_t
TDNFGetDigestForFile(
    const char *filename,
    hash_op *hash,
    uint8_t *digest
    )
{
    uint32_t dwError = 0;
    int fd = -1;
    char buf[BUFSIZ] = {0};
    int length = 0;
    EVP_MD_CTX *ctx = NULL;
    const EVP_MD *digest_type = NULL;
    unsigned int digest_length = 0;

    if (IsNullOrEmptyString(filename) || !hash || !digest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        pr_err("ERROR: Checksum validating (%s) FAILED\n", filename);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    digest_type = EVP_get_digestbyname(hash->hash_type);

    if (!digest_type)
    {
        pr_err("Unknown message digest %s\n", hash->hash_type);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ctx = EVP_MD_CTX_create();
    if (!ctx)
    {
        pr_err("Context Create Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = EVP_DigestInit_ex(ctx, digest_type, NULL);
    if (!dwError)
    {
        pr_err("Digest Init Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        /* MD5 is not approved in FIPS mode. So, overrriding
           the dwError to show the right error to the user */
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
        if (EVP_default_properties_is_fips_enabled(NULL) && !strcasecmp(hash->hash_type, "md5"))
#else
        if (FIPS_mode() && !strcasecmp(hash->hash_type, "md5"))
#endif
        {
            dwError = ERROR_TDNF_FIPS_MODE_FORBIDDEN;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((length = read(fd, buf, BUFSIZ - 1)) > 0)
    {
        dwError = EVP_DigestUpdate(ctx, buf, length);
        if (!dwError)
        {
            pr_err("Digest Update Failed\n");
            dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        memset(buf, 0, BUFSIZ);
    }

    if (length == -1)
    {
        pr_err("Error: Checksum validating (%s) FAILED\n", filename);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = EVP_DigestFinal_ex(ctx, digest, &digest_length);
    if (!dwError)
    {
        pr_err("Digest Final Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;

cleanup:
    if (fd >= 0)
    {
        close(fd);
    }
    if (ctx)
    {
        EVP_MD_CTX_destroy(ctx);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFCheckHash(
    const char *filename,
    const unsigned char *digest,
    int type
    )
{

    uint32_t dwError = 0;
    uint8_t digest_from_file[EVP_MAX_MD_SIZE] = {0};
    hash_op *hash = NULL;

    if (IsNullOrEmptyString(filename) ||
       !digest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (type  < TDNF_HASH_MD5 || type >= TDNF_HASH_SENTINEL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hash = hash_ops + type;

    dwError = TDNFGetDigestForFile(filename, hash, digest_from_file);
    BAIL_ON_TDNF_ERROR(dwError);

    if (memcmp(digest_from_file, digest, hash->length))
    {
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    if (!IsNullOrEmptyString(filename))
    {
        pr_err("Error: Validating Checksum (%s) FAILED (digest mismatch)\n", filename);
    }
    goto cleanup;
}

/* Returns nonzero if hex_digest is properly formatted; that is each
   letter is in [0-9A-Za-z] and the length of the string equals to the
   result length of digest * 2. */
uint32_t
TDNFCheckHexDigest(
    const char *hex_digest,
    int digest_length
    )
{
    int i = 0;

    if(IsNullOrEmptyString(hex_digest) ||
       (digest_length <= 0))
    {
        return 0;
    }

    for(i = 0; hex_digest[i]; ++i)
    {
        if(!isxdigit(hex_digest[i]))
        {
            return 0;
        }
    }

    return digest_length * 2 == i;
}

uint32_t
TDNFHexToUint(
    const char *hex_digest,
    unsigned char *uintValue
    )
{
    uint32_t dwError = 0;
    char buf[3] = {0};
    unsigned long val = 0;

    if(IsNullOrEmptyString(hex_digest) ||
       !uintValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    buf[0] = hex_digest[0];
    buf[1] = hex_digest[1];

    errno = 0;
    val = strtoul(buf, NULL, 16);
    if(errno)
    {
        pr_err("Error: strtoul call failed\n");
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    *uintValue = (unsigned char)(val&0xff);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFChecksumFromHexDigest(
    const char *hex_digest,
    unsigned char *ppdigest
    )
{
    uint32_t dwError = 0;
    unsigned char *pdigest = NULL;
    size_t i = 0;
    size_t len = 0;
    unsigned char uintValue = 0;

    if(IsNullOrEmptyString(hex_digest) ||
       !ppdigest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    len = strlen(hex_digest);

    dwError = TDNFAllocateMemory(1, len/2, (void **)&pdigest);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i < len; i += 2)
    {
        dwError = TDNFHexToUint(hex_digest + i, &uintValue);
        BAIL_ON_TDNF_ERROR(dwError);

        pdigest[i>>1] = uintValue;
    }
    memcpy( ppdigest, pdigest, len>>1 );

cleanup:
    TDNF_SAFE_FREE_MEMORY(pdigest);
    return dwError;

error:
    goto cleanup;
}

/* return true if str is a valid reponame */
int TDNFStrIsValidRepoName(const char *str)
{
    /* should start with alnum or '@' */
    /* '@ as a first character to allow "@System" and "@cmdline" */
    if (!str || !*str || !(isalnum(*str) || (*str == '@')))
        return 0;
    str++;
    while(*str && (isalnum(*str) || *str == '-' || *str == '_' || *str == '.'))
        str++;
    return *str == 0;
}
