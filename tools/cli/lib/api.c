/*
 * Copyright (C) 2017-2022 VMware, Inc. All Rights Reserved.
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
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

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
    if (pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_NO_MATCH)
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
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
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
            pr_crit("%-20s%-40s%-10s\n", "repo id", "repo name", "status");
        }
        for(pRepo = pRepoList; pRepo; pRepo = pRepo->pNext)
        {
            pr_crit(
                "%-20s%-40s%-10s\n",
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
    PTDNF_REPOSYNC_ARGS pReposyncArgs;

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
            int j;
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
                        jd_map_add_string(jd_entry, "Time", szTime);
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
    struct json_dump *jd = NULL;
    struct json_dump *jd_pkg = NULL;

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

    if (pCmdArgs->nJsonOutput)
    {
        jd = jd_create(0);
        CHECK_JD_NULL(jd);

        CHECK_JD_RC(jd_list_start(jd));

    	for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            jd_pkg = jd_create(0);
            CHECK_JD_NULL(jd_pkg);

            jd_map_start(jd_pkg);
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
        for(dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            pPkg = &pPkgInfo[dwIndex];
            pr_crit("%*s\r", 80, pPkg->pszRepoName);
            pr_crit("%*s-%s\r", 50, pPkg->pszVersion, pPkg->pszRelease);
            pr_crit("%s.%s", pPkg->pszName, pPkg->pszArch);
            pr_crit("\n");
        }
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, dwCount);
    }
    return dwError;

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_pkg);
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

