/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : main.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

int main(int argc, char* argv[])
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    TDNF_CLI_CMD_MAP arCmdMap[] = 
    {
        {"autoerase",          TDNFCliAutoEraseCommand},
        {"autoremove",         TDNFCliAutoEraseCommand},
        {"check-local",        TDNFCliCheckLocalCommand},
        {"check-update",       TDNFCliCheckUpdateCommand},
        {"clean",              TDNFCliCleanCommand},
        {"count",              TDNFCliCountCommand},
        {"distro-sync",        TDNFCliDistroSyncCommand},
        {"downgrade",          TDNFCliDowngradeCommand},
        {"erase",              TDNFCliEraseCommand},
        {"help",               TDNFCliHelpCommand},
        {"info",               TDNFCliInfoCommand},
        {"install",            TDNFCliInstallCommand},
        {"list",               TDNFCliListCommand},
        {"makecache",          TDNFCliMakeCacheCommand},
        {"provides",           TDNFCliProvidesCommand},
        {"whatprovides",       TDNFCliProvidesCommand},
        {"reinstall",          TDNFCliReinstallCommand},
        {"remove",             TDNFCliEraseCommand},
        {"repolist",           TDNFCliRepoListCommand},
        {"search",             TDNFCliSearchCommand},
        {"update",             TDNFCliUpgradeCommand},
        {"update-to",          TDNFCliUpgradeCommand},
        {"upgrade",            TDNFCliUpgradeCommand},
        {"upgrade-to",         TDNFCliUpgradeCommand},
        {"updateinfo",         TDNFCliUpdateInfoCommand},
    };
    int nCommandCount = sizeof(arCmdMap)/sizeof(TDNF_CLI_CMD_MAP);
    const char* pszCmd = NULL;
    PTDNF pTdnf = NULL;
    int nFound = 0;

    dwError = TDNFCliParseArgs(argc, argv, &pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //If --version, show version and exit
    if(pCmdArgs->nShowVersion)
    {
        TDNFCliShowVersion();
    }
    else if(pCmdArgs->nShowHelp)
    {
        TDNFCliShowHelp();
    }
    else if(pCmdArgs->nCmdCount > 0)
    {
        pszCmd = pCmdArgs->ppszCmds[0];
        while(nCommandCount > 0)
        {
            --nCommandCount;
            if(!strcmp(pszCmd, arCmdMap[nCommandCount].pszCmdName))
            {
                nFound = 1;

                if(!strcmp(pszCmd, "makecache"))
                {
                    pCmdArgs->nRefresh = 1;
                }

                dwError = TDNFOpenHandle(pCmdArgs, &pTdnf);
                BAIL_ON_CLI_ERROR(dwError);

                dwError = arCmdMap[nCommandCount].pFnCmd(pTdnf, pCmdArgs);
                BAIL_ON_CLI_ERROR(dwError);
                break;
            }
        };
        if(!nFound)
        {
            TDNFCliShowNoSuchCommand(pszCmd);
        }
    }
    else
    {
        TDNFCliShowUsage();
    }

cleanup:
    if(pTdnf)
    {
        TDNFCloseHandle(pTdnf);
    }
    if(pCmdArgs)
    {
        TDNFFreeCmdArgs(pCmdArgs);
    }
    return dwError;

error:
    PrintError(dwError);
    goto cleanup;
}

uint32_t
TDNFCliCleanCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    TDNF_CLEANTYPE nCleanType = CLEANTYPE_NONE;
    PTDNF_CLEAN_INFO pTDNFCleanInfo = NULL;
    char** ppszReposUsed = NULL;

    dwError = TDNFCliParseCleanArgs(pCmdArgs, &nCleanType);
    BAIL_ON_CLI_ERROR(dwError);
  
    dwError = TDNFClean(pTdnf, nCleanType, &pTDNFCleanInfo);
    BAIL_ON_CLI_ERROR(dwError);

    //Print clean info
    printf("Cleaning repos:");
    ppszReposUsed = pTDNFCleanInfo->ppszReposUsed;
    while(*ppszReposUsed)
    {
        printf(" %s", *ppszReposUsed);
        ++ppszReposUsed;
    }

    printf("\n");

    if(pTDNFCleanInfo->nCleanAll)
    {
        printf("Cleaning up everything\n");
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
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    uint32_t unCount = 0;
  
    dwError = TDNFCountCommand(pTdnf, &unCount);
    BAIL_ON_CLI_ERROR(dwError);

    printf("Package count = %d\n", unCount);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    uint32_t unCount = 0;
    uint32_t dwIndex = 0;
    PTDNF_LIST_ARGS pListArgs = NULL;

    dwError = TDNFCliParseListArgs(pCmdArgs, &pListArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFList(
                  pTdnf,
                  pListArgs->nScope,
                  pListArgs->ppszPackageNameSpecs,
                  &pPkgInfo,
                  &unCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < unCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];
        printf("%*s\r", 80, pPkg->pszRepoName);
        printf("%*s-%s\r", 50, pPkg->pszVersion, pPkg->pszRelease);
        printf("%s.%s", pPkg->pszName, pPkg->pszArch);
        printf("\n");
    }

cleanup:
    if(pListArgs)
    {
        TDNFFreeListArgs(pListArgs);
    }
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, unCount);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
  
    char* pszFormattedSize = NULL;

    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    PTDNF_LIST_ARGS pInfoArgs = NULL;

    uint32_t unCount = 0;
    uint32_t dwIndex = 0;
    uint32_t unTotalSize = 0;

    dwError = TDNFCliParseInfoArgs(pCmdArgs, &pInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFInfo(
                  pTdnf,
                  pInfoArgs->nScope,
                  pInfoArgs->ppszPackageNameSpecs,
                  &pPkgInfo,
                  &unCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < unCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];

        printf("Name          : %s\n", pPkg->pszName);
        printf("Arch          : %s\n", pPkg->pszArch);
        printf("Epoch         : %d\n", pPkg->dwEpoch);
        printf("Version       : %s\n", pPkg->pszVersion);
        printf("Release       : %s\n", pPkg->pszRelease);
        printf("Install Size  : %s (%u)\n", pPkg->pszFormattedSize, pPkg->dwInstallSizeBytes);
        printf("Repo          : %s\n", pPkg->pszRepoName);
        printf("Summary       : %s\n", pPkg->pszSummary);
        printf("URL           : %s\n", pPkg->pszURL);
        printf("License       : %s\n", pPkg->pszLicense);
        printf("Description   : %s\n", pPkg->pszDescription);

        printf("\n");

        unTotalSize += pPkg->dwInstallSizeBytes;
    }
  
    dwError = TDNFUtilsFormatSize(unTotalSize, &pszFormattedSize);
    BAIL_ON_CLI_ERROR(dwError);
  
    printf("\nTotal Size: %s (%u)\n", pszFormattedSize, unTotalSize);

cleanup:
    if(pInfoArgs)
    {
        TDNFFreeListArgs(pInfoArgs);
    }
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, unCount);
    }
    TDNF_CLI_SAFE_FREE_MEMORY(pszFormattedSize);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliRepoListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;

    dwError = TDNFCliParseRepoListArgs(pCmdArgs, &nFilter);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFRepoList(pTdnf, nFilter, &pRepos);
    BAIL_ON_CLI_ERROR(dwError);

    pReposTemp = pRepos;
    if(pReposTemp)
    {
        printf("%-20s%-40s%-10s\n", "repo id", "repo name", "status");
    }
    while(pReposTemp)
    {
        printf(
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
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t unError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    uint32_t unCount = 0;
    uint32_t unIndex = 0;

    unError = TDNFSearchCommand(pTdnf, pCmdArgs, &pPkgInfo, &unCount);
    BAIL_ON_CLI_ERROR(unError);

    for(unIndex = 0; unIndex < unCount; ++unIndex)
    {
        pPkg = &pPkgInfo[unIndex];
        printf("%s : %s\n", pPkg->pszName, pPkg->pszSummary);
    }

cleanup:
    TDNFFreePackageInfoArray(pPkgInfo, unCount);
    return unError;

error:
    goto cleanup;
}

uint32_t
TDNFCliCheckLocalCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount < 2)
    {
        dwError = ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCheckLocalPackages(pTdnf, pCmdArgs->ppszCmds[1]);
    BAIL_ON_CLI_ERROR(dwError);

    fprintf(stdout, "Check completed without issues\n");

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliProvidesCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkgInfos = NULL;

    if(!pTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pCmdArgs->nCmdCount < 2)
    {
        dwError = ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFProvides(pTdnf, pCmdArgs->ppszCmds[1], &pPkgInfos);
    BAIL_ON_CLI_ERROR(dwError);

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        fprintf(
            stdout,
            "%s-%s-%s.%s : %s\n",
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch,
            pPkgInfo->pszSummary);
        fprintf(stdout, "Repo\t : %s\n", pPkgInfo->pszRepoName);
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
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;
    PTDNF_PKG_INFO pPkg = NULL;
    uint32_t unCount = 0;
    uint32_t dwIndex = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;

    if(!pTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParsePackageArgs(
                  pCmdArgs,
                  &ppszPackageArgs,
                  &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFCheckUpdates(
                  pTdnf,
                  ppszPackageArgs,
                  &pPkgInfo,
                  &unCount);
    BAIL_ON_CLI_ERROR(dwError);

    for(dwIndex = 0; dwIndex < unCount; ++dwIndex)
    {
        pPkg = &pPkgInfo[dwIndex];
        printf("%*s\r", 80, pPkg->pszRepoName);
        printf("%*s-%s\r", 50, pPkg->pszVersion, pPkg->pszRelease);
        printf("%s.%s", pPkg->pszName, pPkg->pszArch);
        printf("\n");
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    if(pPkgInfo)
    {
        TDNFFreePackageInfoArray(pPkgInfo, unCount);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliMakeCacheCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }
    //Empty as refresh flag is set for makecache command
    //and will execute refresh on all enabled repos

    fprintf(stdout, "Metadata cache created.\n");

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintError(
    uint32_t dwErrorCode
    )
{
    uint32_t dwError = 0;
    char* pszError = NULL;

    if(dwErrorCode < ERROR_TDNF_BASE)
    {
        dwError = TDNFCliGetErrorString(dwErrorCode, &pszError);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        dwError = TDNFGetErrorString(dwErrorCode, &pszError);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(dwErrorCode == ERROR_TDNF_CLI_NOTHING_TO_DO)
    {
        dwErrorCode = 0;
    }
    if(dwErrorCode)
    {
        printf("Error(%d) : %s\n", dwErrorCode, pszError);
    }
    else
    {
        printf("%s\n", pszError);
    }

cleanup:
    TDNF_CLI_SAFE_FREE_MEMORY(pszError);
    return dwError;

error:
    printf(
        "Retrieving error string for %d failed with %d\n",
        dwErrorCode,
        dwError);
    goto cleanup;
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

    nCount = sizeof(arErrorDesc)/sizeof(arErrorDesc[0]);

    for(i = 0; i < nCount; i++)
    {
        if (dwErrorCode == arErrorDesc[i].nCode)
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

void
TDNFCliShowVersion(
    )
{
    fprintf(stdout, "%s: %s\n", PACKAGE_NAME, TDNFGetVersion());
}
