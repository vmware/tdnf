/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <ftw.h>
#include "includes.h"

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
    char* pszSizes = "bkMG";
    double dSize = unSize;

    int nIndex = 0;
    int nLimit = strlen(pszSizes);
    double dKiloBytes = 1024.0;
    int nMaxSize = 35;

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

    if(sprintf(pszFormattedSize, "%6.2f%c %lu", dSize, pszSizes[nIndex],
        (unsigned long)unSize) < 0)
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
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    )
{
    if(pPkgInfo)
    {
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszName);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRepoName);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszVersion);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszArch);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszSummary);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszURL);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszLicense);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszDescription);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszFormattedSize);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszRelease);
        TDNF_SAFE_FREE_MEMORY(pPkgInfo->pszLocation);
    }
}

void
TDNFFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    int i = 0;
    if(pSolvedPkgInfo)
    {
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

        if(pSolvedPkgInfo->ppszPkgsNotInstalled)
        {
            while(pSolvedPkgInfo->ppszPkgsNotInstalled[i])
            {
                TDNF_SAFE_FREE_MEMORY(
                    pSolvedPkgInfo->ppszPkgsNotInstalled[i++]);
            }
        }
        TDNF_SAFE_FREE_MEMORY(pSolvedPkgInfo->ppszPkgsNotInstalled);

        if(pSolvedPkgInfo->ppszPkgsNotResolved)
        {
            while(pSolvedPkgInfo->ppszPkgsNotResolved[i])
            {
                TDNF_SAFE_FREE_MEMORY(
                    pSolvedPkgInfo->ppszPkgsNotResolved[i++]);
            }
        }
        TDNF_SAFE_FREE_MEMORY(pSolvedPkgInfo->ppszPkgsNotResolved);
    }
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

void
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
        if (pCmdOpt->nType != CMDOPT_CURL_INIT_CB)
        {
            TDNF_SAFE_FREE_MEMORY(pCmdOpt->pszOptValue);
        }
        else
        {
            pCmdOpt->pfnCurlConfigCB = NULL;
        }
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
    int nIndex = 0;
    if(pCmdArgs)
    {
        for(nIndex = 0; nIndex < pCmdArgs->nCmdCount; ++nIndex)
        {
            TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds[nIndex]);
        }
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->ppszCmds);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszDownloadDir);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszInstallRoot);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszConfFile);
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszReleaseVer);

        if(pCmdArgs->pSetOpt)
        {
            TDNFFreeCmdOpt(pCmdArgs->pSetOpt);
        }
        TDNF_SAFE_FREE_MEMORY(pCmdArgs);
    }
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
        TDNF_SAFE_FREE_MEMORY(pRepo->pszBaseUrl);
        TDNF_SAFE_FREE_MEMORY(pRepo->pszMetaLink);
        TDNF_SAFE_FREE_STRINGARRAY(pRepo->ppszUrlGPGKeys);

        pRepos = pRepo->pNext;
        TDNF_SAFE_FREE_MEMORY(pRepo);
    }
}

void
TDNFFreeCleanInfo(
    PTDNF_CLEAN_INFO pCleanInfo
    )
{
    if(pCleanInfo)
    {
        TDNF_SAFE_FREE_STRINGARRAY(pCleanInfo->ppszReposUsed);
        TDNFFreeMemory(pCleanInfo);
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
        pr_crit("%s", pszQuestion);
        while ((opt = getchar()) == '\n' || opt == '\r');
        opt = tolower(opt);
        if (opt != 'y' && opt != 'n')
        {
            pr_err("Invalid input\n");
            dwError = ERROR_TDNF_INVALID_INPUT;
            BAIL_ON_TDNF_ERROR(dwError);
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

uint32_t TDNFPathFromUri(
    const char* pszKeyUrl,
    char** ppszPath)
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
