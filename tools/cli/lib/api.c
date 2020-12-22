/*
 * Copyright (C) 2017-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

void
TDNFCliFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    TDNFFreeSolvedPackageInfo(pSolvedPkgInfo);
}

uint32_t
TDNFCliGetErrorString(
    uint32_t dwErrorCode,
    char** ppszError
    )
{
    uint32_t dwError = 0;
    char* pszError = NULL;
    int i = 0;
    int nCount = 0;

    TDNF_ERROR_DESC arErrorDesc[] = TDNF_CLI_ERROR_TABLE;

    nCount = ARRAY_SIZE(arErrorDesc);

    for(i = 0; i < nCount; i++)
    {
        if (dwErrorCode == (uint32_t)arErrorDesc[i].nCode)
        {
            dwError = TDNFAllocateString(arErrorDesc[i].pszDesc, &pszError);
            BAIL_ON_CLI_ERROR(dwError);
            break;
        }
    }
    *ppszError = pszError;
cleanup:
    return dwError;
error:
    TDNF_CLI_SAFE_FREE_MEMORY(pszError);
    goto cleanup;
}

uint32_t
TDNFCliCleanCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    TDNF_CLEANTYPE nCleanType = CLEANTYPE_NONE;
    PTDNF_CLEAN_INFO pTDNFCleanInfo = NULL;
    char** ppszReposUsed = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnClean)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseCleanArgs(pCmdArgs, &nCleanType);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnClean(pContext, nCleanType, &pTDNFCleanInfo);
    BAIL_ON_CLI_ERROR(dwError);

    //Print clean info
    pr_info("Cleaning repos:");
    ppszReposUsed = pTDNFCleanInfo->ppszReposUsed;
    while(*ppszReposUsed)
    {
        pr_info(" %s", *ppszReposUsed);
        ++ppszReposUsed;
    }

    pr_info("\n");

    if(pTDNFCleanInfo->nCleanAll)
    {
        pr_info("Cleaning up everything\n");
    }

cleanup:
    if(pTDNFCleanInfo)
    {
        TDNFFreeCleanInfo(pTDNFCleanInfo);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliCountCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;

    UNUSED(pCmdArgs);

    if(!pContext || !pContext->hTdnf || !pContext->pFnCount)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnCount(pContext, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    pr_crit("Package count = %u\n", dwCount);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliListCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    uint32_t dwCount = 0;
    uint32_t dwIndex = 0;
    PTDNF_LIST_ARGS pListArgs = NULL;

    #define MAX_COL_LEN 256
    char szNameAndArch[MAX_COL_LEN] = {0};
    char szVersionAndRelease[MAX_COL_LEN] = {0};

    #define COL_COUNT 3
    //Name.Arch | Version-Release | Repo
    int nColPercents[COL_COUNT] = {55, 25, 15};
    int nColWidths[COL_COUNT] = {0};

    if(!pContext || !pContext->hTdnf || !pContext->pFnList)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseListArgs(pCmdArgs, &pListArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnList(pContext, pListArgs, &pPkgInfo, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];

        memset(szNameAndArch, 0, MAX_COL_LEN);
        if(snprintf(
            szNameAndArch,
            MAX_COL_LEN,
            "%s.%s",
            pPkg->pszName,
            pPkg->pszArch) < 0)
        {
            dwError = errno;
            BAIL_ON_CLI_ERROR(dwError);
        }

        memset(szVersionAndRelease, 0, MAX_COL_LEN);
        if(snprintf(
            szVersionAndRelease,
            MAX_COL_LEN,
            "%s-%s",
            pPkg->pszVersion,
            pPkg->pszRelease) < 0)
        {
            dwError = errno;
            BAIL_ON_CLI_ERROR(dwError);
        }

        pr_crit(
            "%-*s%-*s%*s\n",
            nColWidths[0],
            szNameAndArch,
            nColWidths[1],
            szVersionAndRelease,
            nColWidths[2],
            pPkg->pszRepoName);
    }

cleanup:
    if(pListArgs)
    {
        TDNFCliFreeListArgs(pListArgs);
    }
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliInfoCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    char* pszFormattedSize = NULL;

    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    PTDNF_LIST_ARGS pInfoArgs = NULL;

    uint32_t dwCount = 0;
    uint32_t dwIndex = 0;
    uint64_t dwTotalSize = 0;

    if(!pContext || !pContext->hTdnf || !pContext->pFnInfo)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseInfoArgs(pCmdArgs, &pInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnInfo(pContext, pInfoArgs, &pPkgInfo, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];

        pr_crit("Name          : %s\n", pPkg->pszName);
        pr_crit("Arch          : %s\n", pPkg->pszArch);
        pr_crit("Epoch         : %d\n", pPkg->dwEpoch);
        pr_crit("Version       : %s\n", pPkg->pszVersion);
        pr_crit("Release       : %s\n", pPkg->pszRelease);
        pr_crit("Install Size  : %s (%u)\n", pPkg->pszFormattedSize, pPkg->dwInstallSizeBytes);
        pr_crit("Repo          : %s\n", pPkg->pszRepoName);
        pr_crit("Summary       : %s\n", pPkg->pszSummary);
        pr_crit("URL           : %s\n", pPkg->pszURL);
        pr_crit("License       : %s\n", pPkg->pszLicense);
        pr_crit("Description   : %s\n", pPkg->pszDescription);

        pr_crit("\n");

        dwTotalSize += pPkg->dwInstallSizeBytes;
    }

    dwError = TDNFUtilsFormatSize(dwTotalSize, &pszFormattedSize);
    BAIL_ON_CLI_ERROR(dwError);

    if(dwCount > 0)
    {
        pr_crit("\nTotal Size: %s (%lu)\n", pszFormattedSize, dwTotalSize);
    }

cleanup:
    if(pInfoArgs)
    {
        TDNFCliFreeListArgs(pInfoArgs);
    }
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    TDNF_CLI_SAFE_FREE_MEMORY(pszFormattedSize);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliRepoListCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;

    if(!pContext || !pContext->hTdnf || !pContext->pFnRepoList)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseRepoListArgs(pCmdArgs, &nFilter);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnRepoList(pContext, nFilter, &pRepos);
    BAIL_ON_CLI_ERROR(dwError);

    pReposTemp = pRepos;
    if(pReposTemp)
    {
        pr_crit("%-20s%-40s%-10s\n", "repo id", "repo name", "status");
    }
    while(pReposTemp)
    {
        pr_crit(
            "%-20s%-40s%-10s\n",
            pReposTemp->pszId,
            pReposTemp->pszName,
            pReposTemp->nEnabled ? "enabled" : "disabled");
        pReposTemp = pReposTemp->pNext;
    }

cleanup:
    TDNFFreeRepos(pRepos);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliSearchCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    uint32_t dwIndex = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnSearch)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnSearch(pContext, pCmdArgs, &pPkgInfo, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];
        pr_crit("%s : %s\n", pPkg->pszName, pPkg->pszSummary);
    }

cleanup:
    TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliCheckLocalCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnCheckLocal)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount < 2)
    {
        dwError = ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnCheckLocal(pContext, pCmdArgs->ppszCmds[1]);
    BAIL_ON_CLI_ERROR(dwError);

    pr_crit("Check completed without issues\n");

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliProvidesCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkgInfos = NULL;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnProvides)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount < 2)
    {
        dwError = ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnProvides(pContext,
                                    pCmdArgs->ppszCmds[1],
                                    &pPkgInfos);
    BAIL_ON_CLI_ERROR(dwError);

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        pr_crit("%s-%s-%s.%s : %s\n",
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch,
            pPkgInfo->pszSummary);
        pr_crit("Repo\t : %s\n", pPkgInfo->pszRepoName);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    if(pPkgInfos)
    {
        TDNFFreePackageInfo(pPkgInfos);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliCheckUpdateCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    uint32_t dwCount = 0;
    uint32_t dwIndex = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnCheckUpdate)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParsePackageArgs(
                  pCmdArgs,
                  &ppszPackageArgs,
                  &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnCheckUpdate(pContext,
                                       ppszPackageArgs,
                                       &pPkgInfo,
                                       &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];
        pr_crit("%*s\r", 80, pPkg->pszRepoName);
        pr_crit("%*s-%s\r", 50, pPkg->pszVersion, pPkg->pszRelease);
        pr_crit("%s.%s", pPkg->pszName, pPkg->pszArch);
        pr_crit("\n");
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliMakeCacheCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }
    //Empty as refresh flag is set for makecache command
    //and will execute refresh on all enabled repos

    pr_crit("Metadata cache created.\n");

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliCheckCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnCheck(pContext);
    BAIL_ON_CLI_ERROR(dwError);

    pr_crit("Check completed without issues\n");
cleanup:
    return dwError;

error:
    goto cleanup;
}
