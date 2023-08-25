/*
 * Copyright (C) 2017-2023 VMware, Inc. All Rights Reserved.
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
    uint32_t nCleanType = CLEANTYPE_NONE;

    if(!pContext || !pContext->hTdnf || !pContext->pFnClean)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseCleanArgs(pCmdArgs, &nCleanType);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnClean(pContext, nCleanType);
    BAIL_ON_CLI_ERROR(dwError);

    pr_info("Done.\n");

cleanup:
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

    if (pCmdArgs->nJsonOutput)
    {
        pr_jsonf("%u", dwCount);
    } else {
        pr_crit("Package count = %u\n", dwCount);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
uint32_t
TDNFCliListPackagesPrint(
    PTDNF_PKG_INFO pPkgInfo,
    uint32_t dwCount,
    int nJsonOutput
)
{
    uint32_t dwError = 0;
    uint32_t dwIndex = 0;
    PTDNF_PKG_INFO pPkg = NULL;
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

    #define MAX_COL_LEN 256
    char szNameAndArch[MAX_COL_LEN] = {0};
    char szVersionAndRelease[MAX_COL_LEN] = {0};

    #define LIST_COL_COUNT 3
    //Name.Arch | Version-Release | Repo
    int nColPercents[LIST_COL_COUNT] = {55, 25, 15};
    int nColWidths[LIST_COL_COUNT] = {0};

    if (nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        jd_list_start(jd);

    	for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            CHECK_JD_RC(jd_map_start(jd_pkg));
            pPkg = &pPkgInfo[dwIndex];

            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkg->pszName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Arch", pPkg->pszArch));
            CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkg->pszVersion, pPkg->pszRelease));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Repo", pPkg->pszRepoName));

            CHECK_JD_RC(jd_list_add_child(jd, jd_pkg));
            JD_SAFE_DESTROY(jd_pkg);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        dwError = GetColumnWidths(LIST_COL_COUNT, nColPercents, nColWidths);
        BAIL_ON_CLI_ERROR(dwError);

        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            pPkg = &pPkgInfo[dwIndex];

     	    memset(szNameAndArch, 0, MAX_COL_LEN);
            if(snprintf(
                szNameAndArch,
                MAX_COL_LEN,
                "%s.%s ",
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
                "%s-%s ",
                pPkg->pszVersion,
                pPkg->pszRelease) < 0)
            {
                dwError = errno;
                BAIL_ON_CLI_ERROR(dwError);
            }

            pr_crit(
                "%-*s %-*s %*s\n",
                nColWidths[0],
                szNameAndArch,
                nColWidths[1],
                szVersionAndRelease,
                nColWidths[2],
                pPkg->pszRepoName);
        }
    }

cleanup:
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
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
    uint32_t dwCount = 0;
    PTDNF_LIST_ARGS pListArgs = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnList)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseListArgs(pCmdArgs, &pListArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnList(pContext, pListArgs, &pPkgInfo, &dwCount);
    if (pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFCliListPackagesPrint(pPkgInfo, dwCount, pCmdArgs->nJsonOutput);
    BAIL_ON_CLI_ERROR(dwError);

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

    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnInfo)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseInfoArgs(pCmdArgs, &pInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnInfo(pContext, pInfoArgs, &pPkgInfo, &dwCount);
    if (pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(1024);
        CHECK_JD_NULL(jd);

        CHECK_JD_RC(jd_list_start(jd));

        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            CHECK_JD_RC(jd_map_start(jd_pkg));
            pPkg = &pPkgInfo[dwIndex];

            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkg->pszName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Arch", pPkg->pszArch));
            CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkg->pszVersion, pPkg->pszRelease));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Repo", pPkg->pszRepoName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Url", pPkg->pszURL));
            CHECK_JD_RC(jd_map_add_int(jd_pkg, "InstallSize", pPkg->dwInstallSizeBytes));
            if (pPkg->dwDownloadSizeBytes)
            {
                CHECK_JD_RC(jd_map_add_int(jd_pkg, "DownloadSize", pPkg->dwDownloadSizeBytes));
            }
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Summary", pPkg->pszSummary));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "License", pPkg->pszLicense));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Description", pPkg->pszDescription));

            CHECK_JD_RC(jd_list_add_child(jd, jd_pkg));
            JD_SAFE_DESTROY(jd_pkg);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            pPkg = &pPkgInfo[dwIndex];

            pr_crit("Name          : %s\n", pPkg->pszName);
            pr_crit("Arch          : %s\n", pPkg->pszArch);
            pr_crit("Epoch         : %d\n", pPkg->dwEpoch);
            pr_crit("Version       : %s\n", pPkg->pszVersion);
            pr_crit("Release       : %s\n", pPkg->pszRelease);
            pr_crit("Install Size  : %s (%u)\n", pPkg->pszFormattedSize, pPkg->dwInstallSizeBytes);
            if (pPkg->dwDownloadSizeBytes)
            {
                pr_crit("Download Size  : %s (%u)\n", pPkg->pszFormattedDownloadSize, pPkg->dwDownloadSizeBytes);
            }
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
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
    goto cleanup;
}

uint32_t
TDNFCliRepoListCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepoList = NULL;
    PTDNF_REPO_DATA pRepo = NULL;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;
    struct json_dump *jd = NULL;
    struct json_dump *jd_repo = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnRepoList)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseRepoListArgs(pCmdArgs, &nFilter);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnRepoList(pContext, nFilter, &pRepoList);
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        jd_list_start(jd);

        for(pRepo = pRepoList; pRepo; pRepo = pRepo->pNext)
        {
            jd_repo = jd_create(0);
            CHECK_JD_NULL(jd_repo);
            CHECK_JD_RC(jd_map_start(jd_repo));

            CHECK_JD_RC(jd_map_add_string(jd_repo, "Repo", pRepo->pszId));
            CHECK_JD_RC(jd_map_add_string(jd_repo, "RepoName", pRepo->pszName));
            CHECK_JD_RC(jd_map_add_bool(jd_repo, "Enabled", pRepo->nEnabled));

            CHECK_JD_RC(jd_list_add_child(jd, jd_repo));
            JD_SAFE_DESTROY(jd_repo);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        if(pRepoList)
        {
            pr_crit("%-20s%-41s%-9s\n", "repo id", "repo name", "status");
        }
        for(pRepo = pRepoList; pRepo; pRepo = pRepo->pNext)
        {
            pr_crit(
                "%-19s %-40s %-9s\n",
                pRepo->pszId,
                pRepo->pszName,
                pRepo->nEnabled ? "enabled" : "disabled");
        }
    }

cleanup:
    TDNFFreeRepos(pRepoList);
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_repo);
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
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

    if(!pContext || !pContext->hTdnf || !pContext->pFnSearch)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnSearch(pContext, pCmdArgs, &pPkgInfo, &dwCount);
    if (pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_NO_SEARCH_RESULTS)
    {
        dwError = 0;
    }
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        jd_list_start(jd);

        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            CHECK_JD_RC(jd_map_start(jd_pkg));

            pPkg = &pPkgInfo[dwIndex];

            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkg->pszName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Summary", pPkg->pszSummary));

            CHECK_JD_RC(jd_list_add_child(jd, jd_pkg));
            JD_SAFE_DESTROY(jd_pkg);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            pPkg = &pPkgInfo[dwIndex];
            pr_crit("%s : %s\n", pPkg->pszName, pPkg->pszSummary);
        }
    }

cleanup:
    TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
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
    PTDNF_PKG_INFO pPkg = NULL;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

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

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        jd_list_start(jd);

        for(pPkg = pPkgInfos; pPkg; pPkg = pPkg->pNext)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            CHECK_JD_RC(jd_map_start(jd_pkg));

            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkg->pszName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Arch", pPkg->pszArch));
            CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkg->pszVersion, pPkg->pszRelease));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Summary", pPkg->pszSummary));

            CHECK_JD_RC(jd_list_add_child(jd, jd_pkg));
            JD_SAFE_DESTROY(jd_pkg);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        for(pPkg = pPkgInfos; pPkg; pPkg = pPkg->pNext)
        {
            pr_crit("%s-%s-%s.%s : %s\n",
                pPkg->pszName,
                pPkg->pszVersion,
                pPkg->pszRelease,
                pPkg->pszArch,
                pPkg->pszSummary);
            pr_crit("Repo\t : %s\n", pPkg->pszRepoName);
        }
    }
cleanup:
    if(pPkgInfos)
    {
        TDNFFreePackageInfo(pPkgInfos);
    }
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
    goto cleanup;
}

uint32_t
TDNFCliRepoSyncCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPOSYNC_ARGS pReposyncArgs = NULL;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnRepoSync)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseRepoSyncArgs(pCmdArgs, &pReposyncArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnRepoSync(pContext, pReposyncArgs);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNFCliFreeRepoSyncArgs(pReposyncArgs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliRepoQueryCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs = NULL;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkgInfos = NULL;
    int nCount = 0, i, j, k;
    char **ppszLines = NULL;
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;
    struct json_dump *jd_list = NULL;
    struct json_dump *jd_entry = NULL;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnRepoQuery)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParseRepoQueryArgs(pCmdArgs, &pRepoqueryArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnRepoQuery(pContext, pRepoqueryArgs, &pPkgInfos, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        jd_list_start(jd);

        for (i = 0; i < (int)dwCount; i++)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            CHECK_JD_RC(jd_map_start(jd_pkg));

            pPkgInfo = &pPkgInfos[i];

            CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Nevra", "%s-%s-%s.%s",
                                       pPkgInfo->pszName,
                                       pPkgInfo->pszVersion,
                                       pPkgInfo->pszRelease,
                                       pPkgInfo->pszArch));

            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkgInfo->pszName));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Arch", pPkgInfo->pszArch));
            CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkgInfo->pszVersion, pPkgInfo->pszRelease));
            CHECK_JD_RC(jd_map_add_string(jd_pkg, "Repo", pPkgInfo->pszRepoName));

            if (pPkgInfo->ppszFileList)
            {
                jd_list = jd_create(0);
                CHECK_JD_NULL(jd_list);

                CHECK_JD_RC(jd_list_start(jd_list));

                for (j = 0; pPkgInfo->ppszFileList[j]; j++)
                {
                    CHECK_JD_RC(jd_list_add_string(jd_list, pPkgInfo->ppszFileList[j]));
                }
                CHECK_JD_RC(jd_map_add_child(jd_pkg, "Files", jd_list));
                JD_SAFE_DESTROY(jd_list);
            }
            if (pPkgInfo->ppszDependencies)
            {
                char *strDepKeys[] = {"Provides", "Obsoletes", "Conflicts",
                                      "Requires", "Recommends", "Suggests",
                                      "Supplements", "Enhances", "Depends",
                                      "RequiresPre"};

                jd_list = jd_create(0);
                CHECK_JD_NULL(jd_list);

                jd_list_start(jd_list);

                for (j = 0; pPkgInfo->ppszDependencies[j]; j++)
                {
                    CHECK_JD_RC(jd_list_add_string(jd_list, pPkgInfo->ppszDependencies[j]));
                }
                CHECK_JD_RC(jd_map_add_child(jd_pkg, strDepKeys[pRepoqueryArgs->depKey-1], jd_list));
                JD_SAFE_DESTROY(jd_list);
            }
            if (pPkgInfo->pChangeLogEntries)
            {
                jd_list = jd_create(0);
                CHECK_JD_NULL(jd_list);

                CHECK_JD_RC(jd_list_start(jd_list));

                PTDNF_PKG_CHANGELOG_ENTRY pEntry;
                for (pEntry = pPkgInfo->pChangeLogEntries; pEntry; pEntry = pEntry->pNext)
                {
                    jd_entry = jd_create(0);
                    CHECK_JD_NULL(jd_entry);

                    char szTime[20] = {0};

                    jd_map_start(jd_entry);

                    if (strftime(szTime, 20, "%a %b %d %Y", localtime(&pEntry->timeTime)))
                    {
                        CHECK_JD_RC(jd_map_add_string(jd_entry, "Time", szTime));
                    }
                    CHECK_JD_RC(jd_map_add_string(jd_entry, "Author", pEntry->pszAuthor));
                    CHECK_JD_RC(jd_map_add_string(jd_entry, "Text", pEntry->pszText));

                    CHECK_JD_RC(jd_list_add_child(jd_list, jd_entry));
                    JD_SAFE_DESTROY(jd_entry);
                }
                CHECK_JD_RC(jd_map_add_child(jd_pkg, "ChangeLogs", jd_list));
                JD_SAFE_DESTROY(jd_list);
            }
            if (pPkgInfo->pszSourcePkg)
            {
                CHECK_JD_RC(jd_map_add_string(jd_pkg, "Source", pPkgInfo->pszSourcePkg));
            }

            CHECK_JD_RC(jd_list_add_child(jd, jd_pkg));
            JD_SAFE_DESTROY(jd_pkg);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        for (i = 0; i < (int)dwCount; i++)
        {
            pPkgInfo = &pPkgInfos[i];

            if (pPkgInfo->ppszDependencies)
            {
                for (j = 0; pPkgInfo->ppszDependencies[j]; j++);
                nCount += j;
            }
            else if (pPkgInfo->ppszFileList)
            {
                for (j = 0; pPkgInfo->ppszFileList[j]; j++);
                nCount += j;
            }
            else if (pPkgInfo->pChangeLogEntries)
            {
                PTDNF_PKG_CHANGELOG_ENTRY pEntry;
                for (pEntry = pPkgInfo->pChangeLogEntries; pEntry; pEntry = pEntry->pNext)
                {
                    char szTime[20] = {0};
                    if (strftime(szTime, 20, "%a %b %d %Y", localtime(&pEntry->timeTime)))
                    {
                        pr_crit("%s %s\n%s\n",
                            szTime,
                            pEntry->pszAuthor,
                            pEntry->pszText);
                    }
                    else
                    {
                        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
                        BAIL_ON_CLI_ERROR(dwError);
                    }
                }
            }
            else if (pPkgInfo->pszSourcePkg)
            {
                pr_crit("%s\n", pPkgInfo->pszSourcePkg);
            }
            else
            {
                pr_crit("%s-%s-%s.%s\n",
                    pPkgInfo->pszName,
                    pPkgInfo->pszVersion,
                    pPkgInfo->pszRelease,
                    pPkgInfo->pszArch);
            }
        }

        if (nCount > 0)
        {
            dwError = TDNFAllocateMemory(nCount + 1, sizeof(char *), (void**)&ppszLines);
            BAIL_ON_CLI_ERROR(dwError);
            for (k = 0, i = 0; i < (int)dwCount; i++)
            {
                pPkgInfo = &pPkgInfos[i];

                if (pPkgInfo->ppszDependencies)
                {
                    for (j = 0; pPkgInfo->ppszDependencies[j]; j++)
                    {
                        ppszLines[k++] = pPkgInfo->ppszDependencies[j];
                    }
                }
                else if (pPkgInfo->ppszFileList)
                {
                    for (j = 0; pPkgInfo->ppszFileList[j]; j++)
                    {
                        ppszLines[k++] = pPkgInfo->ppszFileList[j];
                    }
                }
            }

            dwError = TDNFStringArraySort(ppszLines);
            BAIL_ON_CLI_ERROR(dwError);

            for (j = 0; ppszLines[j]; j++)
            {
                if (j == 0 || strcmp(ppszLines[j], ppszLines[j-1]))
                {
                    pr_crit("%s\n", ppszLines[j]);
                }
            }
        }
    }

cleanup:
    if(pPkgInfos)
    {
        TDNFFreePackageInfoArray(pPkgInfos, dwCount);
    }
    TDNF_CLI_SAFE_FREE_MEMORY(ppszLines);
    TDNFCliFreeRepoQueryArgs(pRepoqueryArgs);
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
    JD_SAFE_DESTROY(jd_list);
    JD_SAFE_DESTROY(jd_entry);
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
    int nCheckUpdateCompat = 0;

    if(!pContext || !pContext->hTdnf || !pCmdArgs || !pContext->pFnCheckUpdate)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nCheckUpdateCompat = GlobalGetDnfCheckUpdateCompat();

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

    if (!nCheckUpdateCompat && !pCmdArgs->nJsonOutput)
    {
        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            pPkg = &pPkgInfo[dwIndex];
            pr_crit("%*s\r", 80, pPkg->pszRepoName);
            pr_crit("%*s-%s\r", 50, pPkg->pszVersion, pPkg->pszRelease);
            pr_crit("%s.%s", pPkg->pszName, pPkg->pszArch);
            pr_crit("\n");
        }
    }
    else
    {
        dwError = TDNFCliListPackagesPrint(pPkgInfo, dwCount, pCmdArgs->nJsonOutput);
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    /* yum and dnf return 100 if there are package updates available.
       puppet depends on this behaviour, even with tdnf. */
    if(nCheckUpdateCompat && dwCount > 0 && dwError == 0)
        return ERROR_TDNF_CLI_CHECK_UPDATES_AVAILABLE;
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

    dwError = TDNFCliRefresh(pContext);
    BAIL_ON_CLI_ERROR(dwError);

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

uint32_t
TDNFCliRefresh(
    PTDNF_CLI_CONTEXT pContext)
{
    return TDNFRefresh(pContext->hTdnf);
}

static
uint32_t
TDNFCliHistoryAlter(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_HISTORY_ARGS pHistoryArgs
)
{
    uint32_t dwError = 0;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;

    if(!pContext || !pCmdArgs || !pHistoryArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnHistoryResolve(pContext, pHistoryArgs, &pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    if (pHistoryArgs->nCommand == HISTORY_CMD_INIT)
    {
        /* There is nothing to do here. */
    }
    else if (!(pSolvedPkgInfo->ppszPkgsNotResolved && pSolvedPkgInfo->ppszPkgsNotResolved[0]))
    {
        dwError = TDNFCliAskForAction(pCmdArgs, pSolvedPkgInfo);
        if (pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_OPERATION_ABORTED)
        {
            dwError = 0;
        }
        BAIL_ON_CLI_ERROR(dwError);

        dwError = pContext->pFnAlterHistory(
                    pContext,
                    pSolvedPkgInfo,
                    pHistoryArgs);
        BAIL_ON_CLI_ERROR(dwError);

        if (pCmdArgs->nJsonOutput)
        {
            dwError = TDNFCliPrintActionComplete(pCmdArgs);
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
    else
    {
        char *pszName = NULL;
        int i;
        pr_crit("The following packages could not be resolved:\n\n");
        for (i = 0, pszName = pSolvedPkgInfo->ppszPkgsNotResolved[0];
             pszName;
             i++, pszName = pSolvedPkgInfo->ppszPkgsNotResolved[i])
        {
            pr_crit("%s\n", pszName);
        }
        pr_crit("\n"
                "The package(s) may have been moved out of the enabled repositories since the\n"
                "last time they were installed. You may be able to resolve this by enabling\n"
                "additional repositories.\n");
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    TDNFCliFreeSolvedPackageInfo(pSolvedPkgInfo);
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
TDNFCliHistoryList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_HISTORY_ARGS pHistoryArgs
)
{
    uint32_t dwError = 0;
    PTDNF_HISTORY_INFO pHistoryInfo = NULL;
    PTDNF_HISTORY_INFO_ITEM pItems = NULL;
    struct json_dump *jd = NULL;
    struct json_dump *jd_item = NULL;
    struct json_dump *jd_list_added = NULL;
    struct json_dump *jd_list_removed = NULL;

    if(!pContext || !pCmdArgs || !pHistoryArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnHistoryList(pContext, pHistoryArgs, &pHistoryInfo);
    BAIL_ON_CLI_ERROR(dwError);

    pItems = pHistoryInfo->pItems;

    if (!pCmdArgs->nJsonOutput)
    {
        int nConsoleWidth, nCmdWidth;

        dwError = GetConsoleWidth(&nConsoleWidth);
        BAIL_ON_CLI_ERROR(dwError);

        /* We want to give as much space as possible to the command line.
           All other fields have fixed lengths. */
        /* 4 = ID, 21 = date/time, 9 = '+add/-rem', 7 = spaces */
        nCmdWidth = nConsoleWidth - 4 - 21 - 9 - 7;

        pr_crit("ID   %-*s date/time             +add  / -rem\n", nCmdWidth, "cmd line");
        for (int i = 0; i < pHistoryInfo->nItemCount; i++)
        {
            char szTime[22] = {0};
            strftime(szTime, 22, "%a %b %d %Y %H:%M", localtime(&pItems[i].timeStamp));
            pr_crit("%4d %-*s %-21s +%-4d / -%-4d\n",
                    pItems[i].nId,
                    nCmdWidth, pItems[i].pszCmdLine,
                    szTime,
                    pItems[i].nAddedCount,
                    pItems[i].nRemovedCount);
            if (pHistoryArgs->nInfo)
            {
                if (pItems[i].ppszAddedPkgs && pItems[i].ppszAddedPkgs[0])
                {
                    int j;
                    pr_crit("added: ");
                    for (j = 0; j < pItems[i].nAddedCount - 1; j++)
                    {
                        pr_crit("%s, ", pItems[i].ppszAddedPkgs[j]);
                    }
                    pr_crit("%s", pItems[i].ppszAddedPkgs[j]);
                    pr_crit("\n");
                }
                if (pItems[i].ppszRemovedPkgs && pItems[i].ppszRemovedPkgs[0])
                {
                    int j;
                    pr_crit("removed: ");
                    for (j = 0; j < pItems[i].nRemovedCount - 1; j++)
                    {
                        pr_crit("%s, ", pItems[i].ppszRemovedPkgs[j]);
                    }
                    pr_crit("%s", pItems[i].ppszRemovedPkgs[j]);
                    pr_crit("\n");
                }
                pr_crit("\n");
            }
        }
    }
    else
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        CHECK_JD_RC(jd_list_start(jd));

        for(int i = 0; i < pHistoryInfo->nItemCount; i++)
        {
            jd_item = jd_create(0);
            CHECK_JD_NULL(jd_item);

            CHECK_JD_RC(jd_map_start(jd_item));
            CHECK_JD_RC(jd_map_add_int(jd_item, "Id", pItems[i].nId));
            CHECK_JD_RC(jd_map_add_string(jd_item, "CmdLine", pItems[i].pszCmdLine));
            CHECK_JD_RC(jd_map_add_int64(jd_item, "TimeStamp", pItems[i].timeStamp));
            CHECK_JD_RC(jd_map_add_int(jd_item, "AddedCount", pItems[i].nAddedCount));
            CHECK_JD_RC(jd_map_add_int(jd_item, "RemovedCount", pItems[i].nRemovedCount));
            if (pHistoryArgs->nInfo)
            {
                jd_list_added = jd_create(0);
                CHECK_JD_NULL(jd_list_added);
                jd_list_start(jd_list_added);

                jd_list_removed = jd_create(0);
                CHECK_JD_NULL(jd_list_removed);
                jd_list_start(jd_list_removed);

                if (pItems[i].ppszAddedPkgs && pItems[i].ppszAddedPkgs[0])
                {
                    for (int j = 0; j < pItems[i].nAddedCount; j++)
                    {
                        CHECK_JD_RC(jd_list_add_string(jd_list_added, pItems[i].ppszAddedPkgs[j]));
                    }
                }
                if (pItems[i].ppszRemovedPkgs && pItems[i].ppszRemovedPkgs[0])
                {
                    for (int j = 0; j < pItems[i].nRemovedCount; j++)
                    {
                        CHECK_JD_RC(jd_list_add_string(jd_list_removed, pItems[i].ppszRemovedPkgs[j]));
                    }
                }
                CHECK_JD_RC(jd_map_add_child(jd_item, "Added", jd_list_added));
                JD_SAFE_DESTROY(jd_list_added);

                CHECK_JD_RC(jd_map_add_child(jd_item, "Removed", jd_list_removed));
                JD_SAFE_DESTROY(jd_list_removed);
            }
            CHECK_JD_RC(jd_list_add_child(jd, jd_item));
            JD_SAFE_DESTROY(jd_item);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
cleanup:
    TDNFFreeHistoryInfo(pHistoryInfo);
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_item);
    JD_SAFE_DESTROY(jd_list_added);
    JD_SAFE_DESTROY(jd_list_removed);
    goto cleanup;
}

uint32_t
TDNFCliHistoryCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
)
{
    uint32_t dwError = 0;
    PTDNF_HISTORY_ARGS pHistoryArgs = NULL;

    if(!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = TDNFCliParseHistoryArgs(pCmdArgs, &pHistoryArgs);
    BAIL_ON_CLI_ERROR(dwError);

    if (pHistoryArgs->nCommand == HISTORY_CMD_LIST)
    {
        dwError = TDNFCliHistoryList(pContext, pCmdArgs, pHistoryArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        dwError = TDNFCliHistoryAlter(pContext, pCmdArgs, pHistoryArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    TDNFCliFreeHistoryArgs(pHistoryArgs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliMarkCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    uint32_t nValue = 0;

    if(!pContext || !pContext->hTdnf || !pContext->pFnCount)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount > 1)
    {
        if (strcmp(pCmdArgs->ppszCmds[1], "install") == 0)
        {
            nValue = 0;
        }
        else if (strcmp(pCmdArgs->ppszCmds[1], "remove") == 0)
        {
            nValue = 1;
        }
        else
        {
            pr_crit("unknown action '%s'\n", pCmdArgs->ppszCmds[1]);
            dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
    else
    {
        pr_crit("need action ('install' or 'remove') as argument\n");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount > 2)
    {
        dwError = pContext->pFnMark(pContext, &(pCmdArgs->ppszCmds[2]), nValue);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        pr_crit("need package spec(s) as argument\n");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

