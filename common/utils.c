/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFFileReadAllText(
    const char *pszFileName,
    char **ppszText
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
        TDNF_SAFE_FREE_MEMORY(pRepo->pszUrlGPGKey);

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
